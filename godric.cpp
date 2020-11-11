#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

int main(int argc, char** argv)
{
  namespace bfs = boost::filesystem;
  namespace bpo = boost::program_options;
  bfs::path ipath, opath;
  bpo::options_description desc("Options");
  desc.add_options()
    ("help,h", "print help message")
    ("input,i", bpo::value<bfs::path>(&ipath)->required(), "directory to sort")
    ("output,o", bpo::value<bfs::path>(&opath)->required(), "directory to put sorted files");
  bpo::variables_map vm;
  try
  {
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }
    bpo::notify(vm);
  }
  catch (bpo::error& e)
  {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }

  bfs::create_directories(opath);
  for(auto& p : bfs::directory_iterator(ipath))
  {
    std::vector<std::string> tokens;
    std::stringstream f(p.path().filename().string().c_str());
    std::string tok;
    while (getline(f, tok, '_'))
    {
      tokens.push_back(tok);
    }
    if (4 == tokens.size())
    {
      auto year = tokens[2].substr(tokens[2].length() - 4);
      auto dir = bfs::path(tokens[1]) / year;
      bfs::create_directories(opath / dir);
      bfs::rename(p.path(), opath / dir / p.path().filename());
    }
  }
}
