/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./copy infile outfile\n");
        return 1;
    }

    // remember filenames
    float f = atof(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    
    BITMAPINFOHEADER resize_bi;
        resize_bi = bi;
        int resize_padding = (4 - (resize_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
        resize_bi.biWidth = bi.biWidth * f;
        resize_bi.biHeight = bi.biHeight * f;
        resize_bi.biSizeImage = ((sizeof(RGBTRIPLE) * resize_bi.biWidth) + resize_padding) * abs(resize_bi.biHeight);
    BITMAPFILEHEADER resize_bf;
        resize_bf = bf;
        resize_bf.bfSize = resize_bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&resize_bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&resize_bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    
    int full_line = resize_bi.biWidth * sizeof(RGBTRIPLE);
    RGBTRIPLE triple_array[full_line];
    int counter = 0;
    
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;
    
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                for(int l = 0; l < f; l++)
                {
                    triple_array[counter] = triple;
                    counter++;
                }
        }
        
        for(int m = 0; m < f; m++)
        {
            // write RGB triple to outfile
            fwrite(&triple_array, full_line, 1, outptr);
    
            //adding padding
            for (int k = 0; k < resize_padding; k++)
            {
                fputc(0x00, outptr);
            }
        }
        counter = 0;
        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;