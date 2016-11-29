function [indI, indJ, minDifference] = registration(indX, indY)
    sourceImage = [1 2 3 4 5;
                   1 7 3 4 5;
                   1 2 9 4 5;
                   1 2 3 4 5;
                   1 2 3 4 5;
                   1 2 3 4 5;
                   1 2 3 4 5];
     
     destinationImage = [1 2 3 4 5;
                         1 2 3 4 5;
                         1 2 3 4 5;
                         1 2 3 4 5;
                         1 2 1 7 3;
                         1 2 1 2 9;
                         1 2 1 2 3];
         
     boundH = 3;
     windowSize = 1;
     
     winMinW = max(1, indY - windowSize);
     winMaxW = min(size(destinationImage, 1), indY + windowSize);
     
     winMinH = max(1, indX - windowSize);
     winMaxH = min(size(destinationImage, 1), indX + windowSize);
     
     window = sourceImage(winMinW : winMaxW, winMinH : winMaxH);
     
     indI = indX;
     indJ = indY;
     baseColor = sourceImage(indX, indY);
     
     minDifference = intmax;
     upperLim = max(2, indX - boundH);
     lowerLim = min(size(destinationImage, 1) - 1, indX + boundH);
     for i = upperLim : lowerLim
         for j = 2 : size(destinationImage, 2) - 1
             searchWindow = destinationImage(i - windowSize : i + windowSize, j - windowSize : j + windowSize)
             
             difference = calculateDifference(window, searchWindow);
             if (difference < minDifference)
                 minDifference = difference;
                 indI = i;
                 indJ = j;
             end
         end
     end
     indI
     indJ
     minDifference
     
end

function difference = calculateDifference(source, destination)
    [sw, sh] = size(source);
    [dw, dh] = size(destination);
    
    assert(sw == dw && sh == dh, 'Sizes of the windows do not match');
    
    difference = 0;
    for i = 1 : sw
        for j = 1 : sh
            difference = difference + abs(source(i, j) - destination(i, j));
        end
    end
    
    difference
end