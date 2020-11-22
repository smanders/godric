#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <wx/app.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include <wx/dirctrl.h>
#include <wx/display.h>
#include <wx/filectrl.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h> // for wxEVT_COMMAND_TOGGLEBUTTON_CLICKED
#include <wx/things/toggle.h>

#include "Resources/action_run.hrc"
#include "Resources/filter_add.hrc"
#include "Resources/filter_delete.hrc"
#include "Resources/folder_download.hrc"
#include "Resources/folder_upload.hrc"

class godricApp : public wxApp
{
public:
  godricApp() {}

protected:
  /** Called by wxWidgets to initialize the application.
   * @return true if initialization was successful.
   */
  virtual bool OnInit();
};

class godricFrame : public wxFrame
{
public:
  godricFrame(wxWindow* pParent);
  virtual ~godricFrame();

private:
  godricFrame() = delete; // No support for two-step construction
  void init();
  bool create();
  wxPanel* createToolbarDir();
  wxPanel* createToolbarFilter();
  wxPanel* createToolbarRun();
  void setStatus();
  void populateDirectoryList();
  boost::filesystem::path filterFile(const boost::filesystem::path& file);
  void execute();

private:
  wxAuiManager* m_pAui;
  wxGenericDirCtrl* m_pInputDir;
  wxListBox* m_pListBox;
  wxFileCtrl* m_pOutputDir;
  wxCustomButton* m_pBtnFilter;
  wxSpinCtrl* m_pDelimNum;
};

/** Application instance implementation */
IMPLEMENT_APP(godricApp)

bool godricApp::OnInit()
{
  try
  {
    // Command line parsing is done in wxApp::OnInit
    if (!wxApp::OnInit()) return false;
    // Set vendor and application name
    SetVendorName("smanders");
    SetAppName("godric");
    // Set working directory to location of executable
    wxSetWorkingDirectory(wxPathOnly(wxGetApp().argv[0]));
    // wxConfig
    wxConfig* pCfg = new wxConfig(GetAppName(),
                                  GetVendorName(),
                                  wxEmptyString,
                                  wxEmptyString,
                                  wxCONFIG_USE_LOCAL_FILE);
    wxConfig::Set(pCfg);
    // Frame creation
    auto pFrame = new godricFrame(nullptr);
    pFrame->Show();
    SetTopWindow(pFrame);
  }
  catch (...)
  {
    return false;
  }
  return true;
}

godricFrame::godricFrame(wxWindow* pParent)
  : wxFrame(pParent,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxSize(600, 750),
            wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMINIMIZE_BOX |
              wxMAXIMIZE_BOX | wxCLOSE_BOX),
    m_pAui(nullptr),
    m_pInputDir(nullptr),
    m_pListBox(nullptr),
    m_pOutputDir(nullptr),
    m_pBtnFilter(nullptr),
    m_pDelimNum(nullptr)
{
  init();
  create();
}

godricFrame::~godricFrame()
{
  wxConfig* pCfg = reinterpret_cast<wxConfig*>(wxConfig::Get());
  // store the directories
  pCfg->SetPath("/Directories");
  pCfg->Write("input", m_pInputDir->GetPath());
  pCfg->Write("output", m_pOutputDir->GetDirectory());
  // store the position and size
  if (IsMaximized() || IsIconized()) Iconize(false);
  pCfg->SetPath("/Geometry");
  wxRect rect = GetRect();
  pCfg->Write("size", rect.width << 16 | rect.height);
  pCfg->Write("pos", rect.x << 16 | (rect.y & 0xFFFF));
  if (m_pAui)
  {
    // TODO save perspective?
    // pCfg->Write("auimain", m_pAui->SavePerspective());
    m_pAui->UnInit();
    wxDELETE(m_pAui);
  }
}

void godricFrame::init()
{
  wxConfig* pCfg = reinterpret_cast<wxConfig*>(wxConfig::Get());

  SetTitle(pCfg->GetAppName());

  // TODO icon

  // size and position
  pCfg->SetPath("/Geometry");
  wxRect defaultRect(0, 0, 800, 600);
  long size = pCfg->Read("size", defaultRect.width << 16 | defaultRect.height);
  long pos = pCfg->Read("pos", defaultRect.x << 16 | defaultRect.y);
  wxRect configRect;
  configRect.width = size >> 16;
  configRect.height = size & 0xFFFF;
  configRect.x = pos >> 16;
  short y = pos & 0xFFFF; // if the y pos is negative...
  configRect.y = y;       //  then this will keep it negative

  // wxConfig-reading robustness: make sure size is at least as big as default
  if (configRect.width < defaultRect.width)
    configRect.width = defaultRect.width;
  if (configRect.height < defaultRect.height)
    configRect.height = defaultRect.height;
#if wxUSE_DISPLAY == 1
  // wxConfig-reading robustness: if the position read from wxConfig is on a
  // display no longer connected, then revert to the default position
  int display = wxDisplay::GetFromPoint(wxPoint(configRect.x, configRect.y));
  // NOTE: wxDisplay::GetCount() is 1-based, wxDisplay::GetFromPoint is 0-based
  if (wxNOT_FOUND == display ||
      static_cast<int>(wxDisplay::GetCount()) <= display)
    SetSize(defaultRect);
  else
    SetSize(configRect);
#else
  SetSize(configRect);
#endif
  SetSizeHints(defaultRect.GetSize());

  if (nullptr == wxImage::FindHandler(wxBITMAP_TYPE_PNG))
    wxImage::AddHandler(new wxPNGHandler);

  CreateStatusBar();
}

bool godricFrame::create()
{
  wxConfig* pCfg = reinterpret_cast<wxConfig*>(wxConfig::Get());

  m_pAui = new wxAuiManager();
  m_pAui->SetManagedWindow(this);

  // input directory
  pCfg->SetPath("/Directories");
  m_pInputDir =
    new wxGenericDirCtrl(this,
                         wxID_ANY,
                         pCfg->Read("input", wxDirDialogDefaultFolderStr),
                         wxDefaultPosition,
                         wxSize(200, -1),
                         wxDIRCTRL_DIR_ONLY),
  Bind(wxEVT_DIRCTRL_SELECTIONCHANGED,
       [=](wxTreeEvent& rEvent) {
         if (m_pInputDir) populateDirectoryList();
         rEvent.Skip();
       },
       m_pInputDir->GetId());
  m_pAui->AddPane(m_pInputDir,
                  wxAuiPaneInfo()
                    .Name("inputdir")
                    .Caption("input directory")
                    .Left()
                    .Layer(1)
                    .MinSize(200, 400)
                    .MaximizeButton(false)
                    .CloseButton(false));

  // directory listing
  m_pListBox = new wxListBox(this, wxID_ANY);
  Bind(
    wxEVT_LISTBOX, [=](wxCommandEvent&) { setStatus(); }, m_pListBox->GetId());
  m_pAui->AddPane(
    m_pListBox,
    wxAuiPaneInfo().Name("listbox").Center().Layer(0).CloseButton(false));

  // output directory
  pCfg->SetPath("/Directories");
  m_pOutputDir = new wxFileCtrl(this,
                                wxID_ANY,
                                pCfg->Read("output", wxEmptyString),
                                wxEmptyString,
                                wxEmptyString,
                                wxFC_SAVE);
  m_pAui->AddPane(m_pOutputDir,
                  wxAuiPaneInfo()
                    .Name("outputdir")
                    .Caption("output directory")
                    .Center()
                    .Layer(0)
                    .CloseButton(false)
                    .Hide());

  // toolbar pane info
  wxAuiPaneInfo tbpi;
  tbpi.ToolbarPane().Top();
  tbpi.LeftDockable(false);
  tbpi.RightDockable(false);
  tbpi.BottomDockable(false);
  tbpi.Floatable(false);
  tbpi.Row(1);
  // directory toolbar
  tbpi.Name("ToolbarDir");
  m_pAui->AddPane(createToolbarDir(), tbpi);
  // directory filter
  tbpi.Name("ToolbarFilter");
  m_pAui->AddPane(createToolbarFilter(), tbpi);
  // run toolbar
  tbpi.Name("ToolbarRun");
  m_pAui->AddPane(createToolbarRun(), tbpi.Hide());

  // "commit" all changes made to wxAuiManager
  m_pAui->Update();

  // load any save perspective
  pCfg->SetPath("/Geometry");
  wxString perspective;
  if (pCfg->Read("auimain", &perspective)) m_pAui->LoadPerspective(perspective);

  populateDirectoryList();
  m_pInputDir->SetFocus();
  return true;
}

wxPanel* godricFrame::createToolbarDir()
{
  auto pPanel = new wxPanel(this, wxID_ANY);
  auto pSizer = new wxBoxSizer(wxHORIZONTAL);
  pPanel->SetSizer(pSizer);

  const wxSize BTNSIZE(42, 42);
  const long TGLSTYLE = wxCUSTBUT_TOGGLE | wxCUSTBUT_FLAT;
  wxSizerFlags szrFlags;
  szrFlags.Proportion(0).Border(wxALL, 2).Center();

  // directory button
  auto pBtnDir = new wxCustomButton(
    pPanel,
    wxID_ANY,
    wxBitmap::NewFromPNGData(folder_upload_png, WXSIZEOF(folder_upload_png)),
    wxDefaultPosition,
    BTNSIZE,
    TGLSTYLE);
  pBtnDir->SetToolTip("toggle input/output directory");
  pBtnDir->SetBitmapSelected(wxBitmap::NewFromPNGData(
    folder_download_png, WXSIZEOF(folder_download_png)));
  pSizer->Add(pBtnDir, szrFlags);
  Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
       [=](wxCommandEvent& rEvent) {
         m_pAui->GetPane("inputdir").Show(!rEvent.IsChecked());
         m_pAui->GetPane("listbox").Show(!rEvent.IsChecked());
         m_pAui->GetPane("ToolbarFilter").Show(!rEvent.IsChecked());
         m_pAui->GetPane("outputdir").Show(rEvent.IsChecked());
         m_pAui->GetPane("ToolbarRun").Show(rEvent.IsChecked());
         m_pAui->Update();
         setStatus();
       },
       pBtnDir->GetId());

  pSizer->SetSizeHints(pPanel);
  return pPanel;
}

wxPanel* godricFrame::createToolbarFilter()
{
  auto pPanel = new wxPanel(this, wxID_ANY);
  auto pSizer = new wxBoxSizer(wxHORIZONTAL);
  pPanel->SetSizer(pSizer);

  const wxSize BTNSIZE(42, 42);
  const long TGLSTYLE = wxCUSTBUT_TOGGLE | wxCUSTBUT_FLAT;
  wxSizerFlags szrFlags;
  szrFlags.Proportion(0).Border(wxALL, 2).Center();

  // filter button
  m_pBtnFilter = new wxCustomButton(
    pPanel,
    wxID_ANY,
    wxBitmap::NewFromPNGData(filter_delete_png, WXSIZEOF(filter_delete_png)),
    wxDefaultPosition,
    BTNSIZE,
    TGLSTYLE);
  m_pBtnFilter->SetToolTip("toggle filter");
  m_pBtnFilter->SetBitmapSelected(
    wxBitmap::NewFromPNGData(filter_add_png, WXSIZEOF(filter_add_png)));
  pSizer->Add(m_pBtnFilter, szrFlags);
  Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
       [=](wxCommandEvent&) { populateDirectoryList(); },
       m_pBtnFilter->GetId());

  // delimiters
  auto pDelimTxt = new wxStaticText(pPanel, wxID_STATIC, "Delimiter _");
  pDelimTxt->SetToolTip("delimiter that separates fields");

  m_pDelimNum =
    new wxSpinCtrl(pPanel, wxID_ANY, "4", wxDefaultPosition, wxSize(60, -1));
  m_pDelimNum->SetToolTip("number of delimiters in filenames");
  Bind(wxEVT_SPINCTRL,
       [=](wxSpinEvent&) { populateDirectoryList(); },
       m_pDelimNum->GetId());

  wxSizerFlags sf;
  sf.Center().Border(wxALL, 2);
  pSizer->Add(pDelimTxt, sf);
  pSizer->Add(m_pDelimNum, sf);

  pSizer->SetSizeHints(pPanel);
  return pPanel;
}

wxPanel* godricFrame::createToolbarRun()
{
  auto pPanel = new wxPanel(this, wxID_ANY);
  auto pSizer = new wxBoxSizer(wxHORIZONTAL);
  pPanel->SetSizer(pSizer);

  const wxSize BTNSIZE(42, 42);
  const long BTNSTYLE = wxCUSTBUT_BUTTON | wxCUSTBUT_FLAT;
  wxSizerFlags szrFlags;
  szrFlags.Proportion(0).Border(wxALL, 2).Center();

  // run button
  auto pBtnRun = new wxCustomButton(
    pPanel,
    wxID_ANY,
    wxBitmap::NewFromPNGData(action_run_png, WXSIZEOF(action_run_png)),
    wxDefaultPosition,
    BTNSIZE,
    BTNSTYLE);
  pBtnRun->SetToolTip("run sorting");
  pSizer->Add(pBtnRun, szrFlags);
  Bind(wxEVT_COMMAND_BUTTON_CLICKED,
       [=](wxCommandEvent&) { execute(); },
       pBtnRun->GetId());

  pSizer->SetSizeHints(pPanel);
  return pPanel;
}

void godricFrame::setStatus()
{
  if (!m_pBtnFilter->GetValue() || m_pAui->GetPane("outputdir").IsShown())
  {
    GetStatusBar()->SetStatusText(wxEmptyString);
    return;
  }
  boost::filesystem::path file(
    m_pListBox->GetString(m_pListBox->GetSelection()));
  file = filterFile(file);
  if (!file.empty()) GetStatusBar()->SetStatusText(file.string());
}

void godricFrame::populateDirectoryList()
{
  namespace bfs = boost::filesystem;
  const boost::filesystem::path input(m_pInputDir->GetPath());
  m_pListBox->Clear();
  GetStatusBar()->SetStatusText(wxEmptyString);
  bool filtered = m_pBtnFilter->GetValue();
  m_pAui->GetPane("listbox").Caption(
    filtered ? "input directory files (filtered)"
             : "input directory files (all, no filter)");
  m_pAui->Update();
  try
  {
    for (auto& p : bfs::directory_iterator(input))
    {
      if (bfs::is_regular_file(p.path()) &&
          (!filtered || (filtered && !filterFile(p.path()).empty())))
        m_pListBox->Append(p.path().filename().string());
    }
  }
  catch (...)
  {
  }
}

boost::filesystem::path godricFrame::filterFile(
  const boost::filesystem::path& file)
{
  std::vector<std::string> fields;
  std::string filename(file.filename().string());
  boost::split(fields, filename, [](char c) { return c == '_'; });
  boost::filesystem::path output;
  if (m_pDelimNum->GetValue() == static_cast<int>(fields.size()))
  {
    auto year = fields[2].substr(fields[2].length() - 4);
    output = boost::filesystem::path(fields[1]) / year / file.filename();
  }
  return output;
}

void godricFrame::execute()
{
  namespace bfs = boost::filesystem;
  const bfs::path indir(m_pInputDir->GetPath());
  const bfs::path outdir(m_pOutputDir->GetDirectory());
  try
  {
    bfs::create_directories(outdir);
    for (auto& p : bfs::directory_iterator(indir))
    {
      if (bfs::is_regular_file(p.path()))
      {
        bfs::path rename = filterFile(p.path());
        if (!rename.empty())
        {
          bfs::create_directories(outdir / rename.parent_path());
          bfs::rename(p.path(), outdir / rename);
        }
      }
    }
    auto inputdir = m_pInputDir->GetPath();
    m_pInputDir->ReCreateTree();
    m_pInputDir->SetPath(inputdir);
    m_pOutputDir->SetDirectory(m_pOutputDir->GetDirectory());
    populateDirectoryList();
  }
  catch (...)
  {
  }
}
