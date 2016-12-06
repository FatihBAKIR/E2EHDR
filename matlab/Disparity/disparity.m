% small window = small SNR
% large window = may cover regin with multiple disparities
% scanWindowSize = 9
% searchScale = 3

function [ disparityMatrix ] = disparity( leftIm, rightIm, scan9, search_3 )
    
    matrixL = imread(leftIm);
    matrixR = imread(rightIm);
    
    % for all pixes in left window  
    lastRow = -1;
    rowLen = size(matrixL, 1) - 8;
    colLen = size(matrixL, 2) - 8;
    
    for row = 1: rowLen
        for col = 1: colLen  
            % get left window - current position
            if (lastRow ~= row)
                disp(row);
                lastRow = row;
            end
            
            %get window for left matrix
            window = matrixL(:,col:col+8);
            WL = window(row:row+8,:);
            
            % get right support window stats
            WRwidth = scan9*search_3;            
            WRlargeX = col - (scan9 * ((search_3-1)/2)) - ((scan9-1)/2);  
            WRlargeY = row ;
            
            if (WRlargeX < 1)
                WRlargeX = 1;
            end           
            if (WRlargeX > 1 + size(matrixL, 2) - WRwidth)
                WRlargeX = 1 + size(matrixL, 2) - WRwidth;
            end
            if (WRlargeY < 1)
                WRlargeY = 1;
            end
            if (WRlargeY > 1 + size(matrixL, 1) - WRwidth)
                WRlargeY = 1 + size(matrixL, 1) - WRwidth;
            end   
            
            % get large support window          
            widthL = search_3 * scan9-1;
            tempW = matrixR(:,WRlargeX:WRlargeX+widthL);
            WRlarge = tempW(WRlargeY:WRlargeY+scan9-1,:);

            % for each pixel in large window   
            matchX = 1;
            maxDisp = -9999999;
            arrWidth = (search_3 * scan9) -8;
            xMid = ((arrWidth - 1) / 2) + 1;
            
            for yrCurr = 1 : 1 + (size(WRlarge, 1) -scan9)
                for xrCurr = 1 : 1 + (size(WRlarge, 2) - scan9)
                    % get right search window
                    window = WRlarge(:,xrCurr:xrCurr+8);
                    matRsupp = window(yrCurr:yrCurr+8,:);
                    
                    %Matching cost computation using SDD
                    %convert int16
                    intL = int16(WL);
                    intR = int16(matRsupp);
                    tempDisp = 0 - sumsqr((intL-intR));
                    
                    
                    %Determine X coordinate && Adjust maxDisparity
                    if (tempDisp > maxDisp)
                        maxDisp = tempDisp;
                        matchX = xrCurr;   
                    end
                    
                end
            end 
            xVector = matchX - xMid + (2 * (xMid - matchX));
            mappedVal = (255 / (xMid -1)) * abs(xVector);            
            disparityMatrix(row, col, 1) = uint8(mappedVal);
        end
    end  
    imageOut = 'C:\Users\Berna\Desktop\Disparity\output_im';
    imwrite(disparityMatrix(:,:,1),imageOut,'GIF');
    
end

