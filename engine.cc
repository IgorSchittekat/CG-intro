#include "easy_image.hh"
#include "ini_configuration.hh"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>


img::EasyImage ColorRectangle(const unsigned int width, const unsigned int height)
{
	img::EasyImage image(width, height);
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			img::Color c;
			c.red = (x * 255) / width;
			c.green = (y * 255) / height;
			c.blue = int((float)(x + y) * (float)255 / width) % 255;
			image(x, y) = c;
		}
	}
	return image;
}

img::EasyImage Blocks(const unsigned int width, const unsigned int height, const std::vector<double> &cWhite, const std::vector<double> &cBlack,
	const unsigned int nrXBlocks, const unsigned int ntYBlocks, const bool invC)
{
	img::EasyImage image(width, height);
	const unsigned int widthBlock = width / nrXBlocks;
	const unsigned int heightBlock = height / ntYBlocks;
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			img::Color c1;
			c1.red = cWhite[0] * 255;
			c1.green = cWhite[1] * 255;
			c1.blue = cWhite[2] * 255;

			img::Color c2;
			c2.red = cBlack[0] * 255;
			c2.green = cBlack[1] * 255;
			c2.blue = cBlack[2] * 255;

			int blockX = x / widthBlock;
			int blockY = y / heightBlock;
			if ((blockX + blockY) % 2 == 0)
			{
				if (!invC) image(x, y) = c1;
				else image(x, y) = c2;
			}
			else
			{
				if (!invC) image(x, y) = c2;
				else image(x, y) = c1;
			}
		}
	}
	return image;
}


img::EasyImage generate_image(const ini::Configuration &configuration)
{
	std::string type = configuration["General"]["type"].as_string_or_die();
	if (type == "IntroColorRectangle")
	{
		const unsigned int width = configuration["ImageProperties"]["width"].as_int_or_die();
		const unsigned int height = configuration["ImageProperties"]["height"].as_int_or_die();
		return ColorRectangle(width, height);
	}
	if (type == "IntroBlocks")
	{
		const unsigned int width = configuration["ImageProperties"]["width"].as_int_or_die();
		const unsigned int height = configuration["ImageProperties"]["height"].as_int_or_die();
		const std::vector<double> cWhite = configuration["BlockProperties"]["colorWhite"].as_double_tuple_or_die();
		const std::vector<double> cBlack = configuration["BlockProperties"]["colorBlack"].as_double_tuple_or_die();
		const unsigned int nrXBlocks = configuration["BlockProperties"]["nrXBlocks"].as_int_or_die();
		const unsigned int nrYBlocks = configuration["BlockProperties"]["nrYBlocks"].as_int_or_die();
		const bool invC = configuration["BlockProperties"]["invertColors"].as_bool_or_die();
		return Blocks(width, height, cWhite, cBlack, nrXBlocks, nrYBlocks, invC);

	}
	return img::EasyImage();
}

int main(int argc, char const* argv[])
{
        int retVal = 0;
        try
        {
                for(int i = 1; i < argc; ++i)
                {
                        ini::Configuration conf;
                        try
                        {
                                std::ifstream fin(argv[i]);
                                fin >> conf;
                                fin.close();
                        }
                        catch(ini::ParseException& ex)
                        {
                                std::cerr << "Error parsing file: " << argv[i] << ": " << ex.what() << std::endl;
                                retVal = 1;
                                continue;
                        }

                        img::EasyImage image = generate_image(conf);
                        if(image.get_height() > 0 && image.get_width() > 0)
                        {
                                std::string fileName(argv[i]);
                                std::string::size_type pos = fileName.rfind('.');
                                if(pos == std::string::npos)
                                {
                                        //filename does not contain a '.' --> append a '.bmp' suffix
                                        fileName += ".bmp";
                                }
                                else
                                {
                                        fileName = fileName.substr(0,pos) + ".bmp";
                                }
                                try
                                {
                                        std::ofstream f_out(fileName.c_str(),std::ios::trunc | std::ios::out | std::ios::binary);
                                        f_out << image;

                                }
                                catch(std::exception& ex)
                                {
                                        std::cerr << "Failed to write image to file: " << ex.what() << std::endl;
                                        retVal = 1;
                                }
                        }
                        else
                        {
                                std::cout << "Could not generate image for " << argv[i] << std::endl;
                        }
                }
        }
        catch(const std::bad_alloc &exception)
        {
    		//When you run out of memory this exception is thrown. When this happens the return value of the program MUST be '100'.
    		//Basically this return value tells our automated test scripts to run your engine on a pc with more memory.
    		//(Unless of course you are already consuming the maximum allowed amount of memory)
    		//If your engine does NOT adhere to this requirement you risk losing points because then our scripts will
		//mark the test as failed while in reality it just needed a bit more memory
                std::cerr << "Error: insufficient memory" << std::endl;
                retVal = 100;
        }
        return retVal;
}
