
#include <math.h>
#include "tiff.h"
#include "allocate.h"
#include "randlib.h"
#include "typeutil.h"

void error(char *name);
void apply2DIIRFilter(uint8_t **input, uint8_t **output, int height, int width);

int main(int argc, char **argv)
{
    FILE *fp;
    struct TIFF_img input_img, output_img;

    if (argc != 2)
        error(argv[0]);

    /* open image file */
    if ((fp = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "cannot open file %s\n", argv[1]);
        exit(1);
    }

    /* read image */
    if (read_TIFF(fp, &input_img))
    {
        fprintf(stderr, "error reading file %s\n", argv[1]);
        exit(1);
    }

    /* close image file */
    fclose(fp);

    /* check the type of image data */
    if (input_img.TIFF_type != 'c')
    {
        fprintf(stderr, "error:  image must be 24-bit color\n");
        exit(1);
    }

    /* set up structure for output color image */
    /* Note that the type is 'c' rather than 'g' */
    get_TIFF(&output_img, input_img.height, input_img.width, 'c');

    apply2DIIRFilter(input_img.color[0], output_img.color[0], input_img.height, input_img.width);
    apply2DIIRFilter(input_img.color[1], output_img.color[1], input_img.height, input_img.width);
    apply2DIIRFilter(input_img.color[2], output_img.color[2], input_img.height, input_img.width);

    /* open image file */
    if ((fp = fopen("iir.tif", "wb")) == NULL)
    {
        fprintf(stderr, "cannot open file iir.tif\n");
        exit(1);
    }

    /* write image */
    if (write_TIFF(fp, &output_img))
    {
        fprintf(stderr, "error writing TIFF file %s\n", argv[2]);
        exit(1);
    }

    /* close image file */
    fclose(fp);

    /* de-allocate space which was used for the images */
    free_TIFF(&(input_img));
    free_TIFF(&(output_img));

    return (0);
}

void apply2DIIRFilter(uint8_t **input, uint8_t **output, int height, int width)
{
    double result;
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            if (i - 1 < 0 && j - 1 < 0)
            {
                result = 0.01 * input[i][j];
            }
            else if (i - 1 < 0)
            {
                result = 0.01 * input[i][j] + 0.9 * (output[i][j - 1]);
            }
            else if (j - 1 < 0)
            {
                result = 0.01 * input[i][j] + 0.9 * (output[i - 1][j]);
            }
            else
            {
                result = 0.01 * input[i][j] + 0.9 * (output[i - 1][j] + output[i][j - 1]) - 0.81 * output[i - 1][j - 1];
            }

            // Clip the result to the 0-255 range
            output[i][j] = (uint8_t)(result < 0 ? 0 : (result > 255 ? 255 : result));
        }
    }
}

void error(char *name)
{
    printf("usage:  %s  image.tiff \n\n", name);
    printf("this program reads in a 24-bit color TIFF image.\n");
    printf("It then horizontally filters the green component, adds noise,\n");
    printf("and writes out the result as an 8-bit image\n");
    printf("with the name 'green.tiff'.\n");
    printf("It also generates an 8-bit color image,\n");
    printf("that swaps red and green components from the input image");
    exit(1);
}
