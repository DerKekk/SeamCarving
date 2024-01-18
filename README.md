# Seam Carving

Shortens pictures in width for the specified amount of pixels through the seam carving algorithm implemented in C++. At this point, supported file formats are .png and .jpg. 


## How it works
Seam carving can crop images while keeping important information by determining which parts are important.

Take a look at the following example image:
![example](https://github.com/DerKekk/SeamCarving/assets/87085389/8a7491d2-df82-4193-9167-806b2cb57994)


We will shorten this image by 500 pixels.
First, an energy map is generated using gradient magnitude:
![energy](https://github.com/DerKekk/SeamCarving/assets/87085389/323b682e-3079-4677-94b0-9dd44718859f)
(It is apparent, that on this particular image, important features are largelay on the right, meaning seams will be primarily taken from the left side)


Afterwards, seams are built. The program allows for a limit on how many seams should be built, improving performance at the cost of accuracy.
Seams are built from the top down, by determining the way with the least significant pixels.
Here, 100 seams have been built:
![seams](https://github.com/DerKekk/SeamCarving/assets/87085389/ac61292b-bc32-4190-a6eb-07a247013c17)


After building the seams, the least relevant is picked out and corresponding pixels are removed from the image
The resulting image looks like this:
![output1](https://github.com/DerKekk/SeamCarving/assets/87085389/942eb777-dec8-4596-8107-3920b21593b5)



Of course, this procedure can be applied again.
Shortened by another 600 pixels, the image looks like this:
![output2](https://github.com/DerKekk/SeamCarving/assets/87085389/b4e6a9ab-43a4-4c5d-a8c5-33de3c310d77)

Now, that pretty much the whole left side has been removed, as it had the least significant pixels, the right side will be more affected:
![output](https://github.com/DerKekk/SeamCarving/assets/87085389/fccbdc3f-f060-45fe-a957-1fd9790a390f)
