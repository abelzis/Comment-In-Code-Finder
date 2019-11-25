// CommentFinderInCode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
using std::string;
using std::vector;

namespace fs = std::filesystem;	// shorten namespace


void writeFileName(std::ofstream& out, const string& str, bool& bl)
{
	if (!bl)
		out << "---------------" << str << "---------------\n";
	bl = true;
}

void fillMultilineSymbols(char chr[], const char& a, const char& b)
{
	chr[0] = a;
	chr[1] = b;
}

void searchComments(std::ifstream& inp, std::ofstream& out, const fs::directory_entry& entry)
{
	int out_line_num = 1;

	bool comment_written = false;

	bool is_multiline_comment = false, in_quotes = false;
	while (!inp.eof() && inp.is_open())
	{
		string temp;
		std::getline(inp, temp);

		char multiline_symbol[2];

		string extension = entry.path().extension().string();

		// add any type of multiline comment symbols depending on programming language
		if (extension == ".cshtml")
			fillMultilineSymbols(multiline_symbol, '@', '*');
		else if (extension == ".html")
			fillMultilineSymbols(multiline_symbol, '<', '-');
		else
			fillMultilineSymbols(multiline_symbol, '/', '*');

		int start = 0;
		for (int i = 0; i < temp.size(); i++)
		{
			if (temp[i] == '\"')
				in_quotes = !in_quotes;
			// if file is html, use different rules
			if (extension == ".html")
			{
				if (temp[i] == multiline_symbol[0])
				{
					if (i + 3 < temp.size())
					{
						if (temp[i + 1] == '!' && temp[i + 2] == multiline_symbol[1] && temp[i + 3] == multiline_symbol[1])
						{
							writeFileName(out, entry.path().filename().string(), comment_written);

							is_multiline_comment = true;
							start = i;	// start position is set to comment start position
							out << out_line_num << ". ";
							out_line_num++;
							continue;
						}
					}
				}
				else if (temp[i] == '>')
				{
					if (i - 2 >= 0 && is_multiline_comment)
					{
						if (temp[i - 1] == multiline_symbol[1])
						{
							out << std::string_view(temp.c_str() + start, i + 1 - start) << "\n";
							is_multiline_comment = false;
						}
					}
				}	
			}
			else if (temp[i] == multiline_symbol[0] && !in_quotes)
			{
				// if a simple one line comment like this one
				if (i + 1 < temp.size() && !is_multiline_comment)
				{
					// if triple dash "///", it's xml and we don't care about these
					if (i + 2 < temp.size() && entry.path().extension().string() == ".cs")
						if (temp[i] == '/' && temp[i + 1] == '/' && temp[i + 2] == '/')
							break;
					// if "//"
					if (temp[i + 1] == '/')
					{
						writeFileName(out, entry.path().filename().string(), comment_written);

						out << out_line_num << ". " << std::string_view(temp.c_str() + i, temp.size() - i) << "\n";
						out_line_num++;
						break;
					}
					// if "/*"
					else if (temp[i + 1] == multiline_symbol[1])
					{
						writeFileName(out, entry.path().filename().string(), comment_written);

						is_multiline_comment = true;
						start = i;	// start position is set to comment start position
						out << out_line_num << ". ";
						out_line_num++;
						continue;
					}
					
				}
				// end of the multiline comment "*/"
				if (i - 1 >= 0 && is_multiline_comment)
				{
					if (temp[i - 1] == multiline_symbol[1])
					{
						out << std::string_view(temp.c_str() + start, i + 1 - start) << "\n";
						is_multiline_comment = false;
					}
				}
			}
			// if a multiline comment has no symbols inbetween
			/*
				like this line
			*/
			else if (is_multiline_comment && i + 1 == temp.size())
			{
				out << std::string_view(temp.c_str() + start, temp.size() - start) << "\n";
				break;
			}
		}
	}
}

void getFilePathFromSubfolder(const string& path, std::ofstream& out)
{
	// loop iterates every subfolder and returns all paths to files found
	for (const auto& entry : fs::directory_iterator(path))
	{
		if (entry.is_directory())
			getFilePathFromSubfolder(entry.path().string(), out);
		else if (!entry.is_directory())
		{
			string file_text;
			std::ifstream inp(entry.path().string());

			searchComments(inp, out, entry);
		}
		else
			throw "Unknown file.";
	}
}

int main(int argc, const char *argv[])
{
	if (argc > 1)	// if cmd arguments passed, only try to take the first one
	{
		string init_path = argv[1];

		std::cout << "INPUT: " << argv[1] << "\n";

		std::ofstream out("result.txt");

		try 
		{
			getFilePathFromSubfolder(init_path, out);
			std::cout << "\n\nSuccess. Please refer to output file named \"results.txt\"\n";
		}
		catch (const std::exception & e)
		{
			std::cout << "Error: " << e.what() << "\n";
		}

	}
	else if (argc == 1)	// if no cmd arguments passed
	{
		std::cout << "This program outputs all programming comments from code base of input folder.\n";
		std::cout << "Input example: \"C:\\Users\\NameSurname\\Program123\"\n";
		std::cout << "Please input full path to a folder: ";

		string init_path;

		std::getline(std::cin, init_path);

		std::ofstream out("result.txt");
		
		try
		{
			getFilePathFromSubfolder(init_path, out);
			std::cout << "\n\nSuccess. Please refer to output file named \"results.txt\"\n";
		}
		catch (const std::exception & e)
		{
			std::cout << "Error: " << e.what() << "\n";
		}
	}
}