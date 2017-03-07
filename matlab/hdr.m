img = hdrread('office.hdr');
figure, imshow(img);

image_h =size(img, 1);
image_w =size(img, 2);

I = rgb2gray(img);
figure, imshow(I);
title('Projector');

I = sqrt(I);
Iblur3 = imgaussfilt(I,3.5);
figure, 
imshow(Iblur3);

Z1 = imdivide(img(:,:,1),Iblur3);
Z2 = imdivide(img(:,:,2),Iblur3);
Z3 = imdivide(img(:,:,3),Iblur3);

Z= cat(3, Z1,Z2, Z3);
figure, 
imshow(Z);
title('LCD');

