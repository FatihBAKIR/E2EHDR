//
// Created by fatih on 24.10.2016.
//


#include "profiler.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <thread>

thread_local profiler_data* current_prof = nullptr;

void init_profiler(const char* name)
{
    static thread_local profiler_data root = ([&]{
        profiler_data data {0};
        data.file = "profiler.cpp";
        data.name = name;
        data.parent = nullptr;
        data.cur_begin = std::chrono::high_resolution_clock::now();
        return data;
    })();
    current_prof = &root;
}

boost::property_tree::ptree prof_to_tree(const profiler_data& data)
{
    using boost::property_tree::ptree;

    ptree self;
    self.put("name", data.name);
    self.put("file", data.file);
    self.put("line", data.line);
    self.put("total_duration", data.total_duration);
    self.put("total_hits", data.total_hits);
    self.put("avg_us", data.total_duration * 1.0f / data.total_hits);
    self.put("last_us", data.last_us);
    self.put("min_us", data.min_us);
    self.put("max_us", data.max_us);

    ptree children;

    for (auto i = data.child; i; i = i->sibling)
    {
        children.push_back(std::make_pair("", prof_to_tree(*i)));
    }

    if (children.size())
        self.add_child("children", children);

    return self;
}

void print_tree() {
    using boost::property_tree::ptree;
    using boost::property_tree::read_json;
    using boost::property_tree::write_json;

    auto* cur = current_prof;
    while (cur->parent) cur = cur->parent; //climb up to root

    const auto diff = std::chrono::high_resolution_clock::now() - cur->cur_begin;
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    cur->total_hits = 1;
    cur->total_duration = us;

    ptree pt = prof_to_tree(*cur);

    std::ostringstream buf;
    write_json (std::cout, pt, false);
    std::string json = buf.str();

    std::cout << json;
}
