#include <boost/filesystem.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>

class godric : public wxApp
{
public:
  godric() {}

protected:
  /** Called by wxWidgets to initialize the application.
   * @return true if initialization was successful.
   */
  virtual bool OnInit();

  /** Called by wxWidgets to initialize command line handling.
   *  This method is called before command line parsing begins. It gives you a
   *  chance to add command line switches and options.
   */
  virtual void OnInitCmdLine(wxCmdLineParser& rParser);

  /** Called by wxWidgets to allow this class to handle the command line.
   *  This method is called after the command line has been parsed. It gives
   *  you the chance to do something with the switches and options.
   * @return true to continue and false to exit
   */
  virtual bool OnCmdLineParsed(wxCmdLineParser& rParser);

  /** Called by wxWidgets to begin program execution.
   * @return exit code (this is later supplied to wxApp::OnExit).
   */
  virtual int OnRun();

private:
  void execute();
  boost::filesystem::path m_inputDir;
  boost::filesystem::path m_outputDir;
};

/** Application instance implementation */
IMPLEMENT_APP(godric)

bool godric::OnInit()
{
  try
  {
    // Command line parsing is done in wxApp::OnInit
    if (!wxApp::OnInit()) return false;
    SetAppName("godric");
    wxSetWorkingDirectory(wxPathOnly(wxGetApp().argv[0]));
  }
  catch (...)
  {
    return false;
  }
  return true;
}

void godric::OnInitCmdLine(wxCmdLineParser& rParser)
{
  wxApp::OnInitCmdLine(rParser);
  static const wxCmdLineEntryDesc cmdLineDesc[] = {
    // Add command line switches, options, and parameters here.
    // See wxCmdLineParser for details.
    {wxCMD_LINE_OPTION,
     "i",
     "input",
     "input directory",
     wxCMD_LINE_VAL_NONE,
     0},
    {wxCMD_LINE_OPTION,
     "o",
     "output",
     "output directory",
     wxCMD_LINE_VAL_NONE,
     0},
    {wxCMD_LINE_NONE,
     nullptr,
     nullptr,
     nullptr,
     wxCMD_LINE_VAL_NONE,
     0} // used to terminate the list
  };
  rParser.SetDesc(cmdLineDesc);
  // refuse '/' as parameter starter or cannot use "/path" style paths
  rParser.SetSwitchChars("-");
}

bool godric::OnCmdLineParsed(wxCmdLineParser& rParser)
{
  // Give default processing (-?, --help and --verbose) the chance to do
  // something.
  wxApp::OnCmdLineParsed(rParser);
  /////////////////////////////////////////////////////////
  // Get input directory from commandline or from dialog
  wxString input;
  if (!rParser.Found("input", &input) && !wxFileName::DirExists(input))
  {
    wxDirDialog open(GetTopWindow(),
                     "Choose input directory",
                     wxEmptyString,
                     wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (wxID_OK != open.ShowModal()) return false;
    input = open.GetPath();
  }
  wxFileName inputDir(input);
  if (!inputDir.IsAbsolute()) inputDir.MakeAbsolute();
  m_inputDir = boost::filesystem::path(inputDir.GetFullPath());
  /////////////////////////////////////////////////////////
  // Get output directory from commandline or from dialog
  wxString output;
  if (!rParser.Found("output", &output))
  {
    wxDirDialog save(GetTopWindow(),
                     "Choose output directory",
                     wxEmptyString,
                     wxDD_DEFAULT_STYLE);
    if (wxID_OK != save.ShowModal()) return false;
    output = save.GetPath();
  }
  wxFileName outputDir(output);
  if (!outputDir.IsAbsolute()) outputDir.MakeAbsolute();
  m_outputDir = boost::filesystem::path(outputDir.GetFullPath());
  return true;
}

int godric::OnRun()
{
  try
  {
    execute();
  }
  catch (...)
  {
  }
  return -1;
}

void godric::execute()
{
  namespace bfs = boost::filesystem;
  bfs::create_directories(m_outputDir);
  for (auto& p : bfs::directory_iterator(m_inputDir))
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
      bfs::create_directories(m_outputDir / dir);
      bfs::rename(p.path(), m_outputDir / dir / p.path().filename());
    }
  }
}
