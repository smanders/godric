#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <wx/app.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include <wx/dirctrl.h>
#include <wx/display.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h> // for wxEVT_COMMAND_TOGGLEBUTTON_CLICKED
#include <wx/things/toggle.h>
#include <wx/thread.h>

#include "Resources/action_run.hrc"
#include "Resources/filter_add.hrc"
#include "Resources/filter_delete.hrc"
#include "Resources/folder_download.hrc"

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

class godricFrame : public wxFrame, public wxThreadHelper
{
public:
  godricFrame(wxWindow* pParent);
  virtual ~godricFrame();

private:
  godricFrame() = delete; // No support for two-step construction
  void onClose(wxCloseEvent& rEvent);
  void init();
  bool create();
  wxPanel* createToolbar();
  void setStatus();
  void populateDirectoryList();
  boost::filesystem::path filterFile(const boost::filesystem::path& file);
  void startWorker();
  void startThread();
  virtual wxThread::ExitCode Entry();
  void onThreadUpdate(wxThreadEvent& rEvent);
  void endWorker();

private:
  wxAuiManager* m_pAui;
  wxGenericDirCtrl* m_pInputDir;
  wxListBox* m_pListBox;
  wxTextCtrl* m_pOutputDir;
  wxCustomButton* m_pBtnFilter;
  wxSpinCtrl* m_pFieldNum;
  wxTextCtrl* m_pTxtFilter;
  wxProgressDialog* m_pProgress;
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
            wxSize(850, 750),
            wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMINIMIZE_BOX |
              wxMAXIMIZE_BOX | wxCLOSE_BOX),
    m_pAui(nullptr),
    m_pInputDir(nullptr),
    m_pListBox(nullptr),
    m_pOutputDir(nullptr),
    m_pBtnFilter(nullptr),
    m_pFieldNum(nullptr),
    m_pTxtFilter(nullptr),
    m_pProgress(nullptr)
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
  pCfg->Write("output", m_pOutputDir->GetValue());
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

void godricFrame::onClose(wxCloseEvent&)
{
  if (GetThread() && GetThread()->IsRunning()) GetThread()->Wait();
  Destroy();
}

void godricFrame::init()
{
  wxConfig* pCfg = reinterpret_cast<wxConfig*>(wxConfig::Get());

  SetTitle(pCfg->GetAppName());

  // TODO icon

  // size and position
  pCfg->SetPath("/Geometry");
  wxRect defaultRect(wxDefaultPosition, GetSize());
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

  // toolbar
  m_pAui->AddPane(createToolbar(),
                  wxAuiPaneInfo()
                    .ToolbarPane()
                    .Top()
                    .Name("Toolbar")
                    .LeftDockable(false)
                    .RightDockable(false)
                    .BottomDockable(false)
                    .Floatable(false)
                    .Row(1));

  // "commit" all changes made to wxAuiManager
  m_pAui->Update();

  // load any save perspective
  pCfg->SetPath("/Geometry");
  wxString perspective;
  if (pCfg->Read("auimain", &perspective)) m_pAui->LoadPerspective(perspective);

  Bind(wxEVT_THREAD, &godricFrame::onThreadUpdate, this);
  Bind(wxEVT_CLOSE_WINDOW, &godricFrame::onClose, this);

  populateDirectoryList();
  m_pInputDir->SetFocus();
  return true;
}

wxPanel* godricFrame::createToolbar()
{
  auto pPanel = new wxPanel(this, wxID_ANY);
  auto pSizer = new wxBoxSizer(wxHORIZONTAL);
  pPanel->SetSizer(pSizer);

  const wxSize BTNSIZE(42, 42);
  const long BTNSTYLE = wxCUSTBUT_BUTTON | wxCUSTBUT_FLAT;
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
  auto pDelimTxt = new wxStaticText(pPanel, wxID_STATIC, "Delimiter '_'");
  pDelimTxt->SetToolTip("delimiter that separates fields");
  pSizer->Add(pDelimTxt, szrFlags);

  m_pFieldNum = new wxSpinCtrl(pPanel,
                               wxID_ANY,
                               "4",
                               wxDefaultPosition,
                               wxSize(35, -1),
                               wxSP_ARROW_KEYS | wxSP_WRAP,
                               /*min=*/1,
                               /*max=*/9);
  m_pFieldNum->SetToolTip(
    "number of fields (separated by delimiters) in filenames");
  pSizer->Add(m_pFieldNum, szrFlags);
  Bind(wxEVT_SPINCTRL,
       [=](wxSpinEvent&) {
         if (m_pBtnFilter->GetValue()) populateDirectoryList();
       },
       m_pFieldNum->GetId());

  // filter string
  m_pTxtFilter = new wxTextCtrl(pPanel,
                                wxID_ANY,
                                "%1:/:$4%2:/:$4%2:_:%1:_:%3",
                                wxDefaultPosition,
                                wxSize(250, -1));
  m_pTxtFilter->SetToolTip("filter string");
  pSizer->Add(m_pTxtFilter, szrFlags);
  Bind(
    wxEVT_TEXT, [=](wxCommandEvent&) { setStatus(); }, m_pTxtFilter->GetId());

  // directory button
  auto pBtnDir =
    new wxCustomButton(pPanel,
                       wxID_ANY,
                       wxBitmap::NewFromPNGData(
                         folder_download_png, WXSIZEOF(folder_download_png)),
                       wxDefaultPosition,
                       BTNSIZE,
                       BTNSTYLE);
  pBtnDir->SetToolTip("select output directory");
  pSizer->Add(pBtnDir, szrFlags);
  Bind(wxEVT_COMMAND_BUTTON_CLICKED,
       [=](wxCommandEvent&) {
         wxDirDialog output(this,
                            "select output directory",
                            m_pOutputDir->GetValue(),
                            wxDD_DEFAULT_STYLE);
         if (wxID_OK == output.ShowModal())
           m_pOutputDir->SetValue(output.GetPath());
       },
       pBtnDir->GetId());

  wxConfig* pCfg = reinterpret_cast<wxConfig*>(wxConfig::Get());
  pCfg->SetPath("/Directories");
  m_pOutputDir = new wxTextCtrl(pPanel,
                                wxID_ANY,
                                pCfg->Read("output", wxEmptyString),
                                wxDefaultPosition,
                                wxSize(250, -1),
                                wxTE_READONLY);
  m_pOutputDir->SetToolTip("output directory");
  pSizer->Add(m_pOutputDir, szrFlags);

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
       [=](wxCommandEvent&) { startWorker(); },
       pBtnRun->GetId());

  pSizer->SetSizeHints(pPanel);
  return pPanel;
}

void godricFrame::setStatus()
{
  if (!m_pBtnFilter->GetValue())
  {
    GetStatusBar()->SetStatusText("no filter on selected file");
    return;
  }
  boost::filesystem::path file(
    m_pListBox->GetString(m_pListBox->GetSelection()));
  file = filterFile(file);
  if (!file.empty())
    GetStatusBar()->SetStatusText(file.string());
  else
    GetStatusBar()->SetStatusText("filter error");
}

void godricFrame::populateDirectoryList()
{
  m_pListBox->Clear();
  GetStatusBar()->SetStatusText(wxEmptyString);
  bool filtered = m_pBtnFilter->GetValue();
  m_pAui->GetPane("listbox").Caption(
    filtered ? "input directory files (filtered)"
             : "input directory files (all, no filter)");
  m_pAui->Update();
  startThread();
}

int ctoi(const std::string& fmt, int offset)
{
  int ret;
  std::stringstream s;
  s << fmt[offset];
  s >> ret;
  return ret;
}

boost::filesystem::path godricFrame::filterFile(
  const boost::filesystem::path& file)
{
  std::vector<std::string> fields;
  boost::split(
    fields, file.filename().string(), boost::algorithm::is_any_of("_"));
  boost::filesystem::path output;
  try
  {
    if (m_pFieldNum->GetValue() == static_cast<int>(fields.size()))
    {
      std::vector<std::string> formats, parts;
      std::string formatString(m_pTxtFilter->GetValue());
      boost::split(formats, formatString, boost::algorithm::is_any_of(":"));
      std::string part;
      for (const auto& fmt : formats)
      {
        switch (fmt[0])
        {
        case '%':
          part += fields.at(ctoi(fmt, 1));
          break;
        case '$':
          if ('%' == fmt[2])
          {
            auto end = ctoi(fmt, 1);
            auto str = fields.at(ctoi(fmt, 3));
            part += str.substr(str.length() - end);
          }
          break;
        case '^':
          if ('%' == fmt[2])
          {
            auto beg = ctoi(fmt, 1);
            auto str = fields.at(ctoi(fmt, 3));
            part += str.substr(0, beg);
          }
          break;
        case '_':
          part += "_";
          break;
        case '/':
          parts.push_back(part);
          part.clear();
          break;
        }
      }
      if (!part.empty()) parts.push_back(part);
      for (const auto& part : parts)
        output /= part;
    }
  }
  catch (...)
  {
  }
  return output;
}

void godricFrame::startWorker()
{
  m_pProgress =
    new wxProgressDialog("sorting progress",
                         "wait until complete or press [Cancel]",
                         100,
                         this,
                         wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME |
                           wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

  namespace bfs = boost::filesystem;
  const bfs::path indir(m_pInputDir->GetPath());
  auto count = std::count_if(
    bfs::directory_iterator(indir),
    bfs::directory_iterator(),
    static_cast<bool (*)(const bfs::path&)>(bfs::is_regular_file));
  m_pProgress->SetRange(count);

  const bfs::path outdir(m_pOutputDir->GetValue());
  bfs::create_directories(outdir);

  startThread();
}

void godricFrame::startThread()
{
  if (GetThread() && GetThread()->IsRunning()) GetThread()->Wait();
  if (wxTHREAD_NO_ERROR != CreateThread(wxTHREAD_JOINABLE))
  {
    wxLogError("could not create the worker thread");
    return;
  }
  if (wxTHREAD_NO_ERROR != GetThread()->Run())
  {
    wxLogError("could not run the worker thread");
    return;
  }
}

wxThread::ExitCode godricFrame::Entry()
{
  namespace bfs = boost::filesystem;
  const bfs::path indir(m_pInputDir->GetPath());
  const bfs::path outdir(m_pOutputDir->GetValue());
  bool filtered = m_pBtnFilter->GetValue();
  try
  {
    int num = 0;
    for (auto& p : bfs::directory_iterator(indir))
    {
      if (bfs::is_regular_file(p.path()))
      {
        wxThreadEvent evt;
        evt.SetString(wxEmptyString);
        evt.SetInt(num);
        if (m_pProgress)
        {
          bfs::path rename = filterFile(p.path());
          if (!rename.empty())
          {
            bfs::create_directories(outdir / rename.parent_path());
            bfs::rename(p.path(), outdir / rename);
          }
          if (0 == num % 1000) QueueEvent(evt.Clone());
        }
        else if (num < 150)
        {
          if (!filtered || (filtered && !filterFile(p.path()).empty()))
          {
            evt.SetString(p.path().filename().string());
            QueueEvent(evt.Clone());
          }
        }
        else if (150 == num)
        {
          evt.SetString("only first 150 files displayed or filtered");
          QueueEvent(evt.Clone());
        }
        else
          break;
        if ((0 == (++num) % 10) && GetThread()->TestDestroy()) break;
      }
    }
  }
  catch (...)
  {
  }
  wxThreadEvent evt;
  evt.SetString(wxEmptyString);
  evt.SetInt(-1);
  QueueEvent(evt.Clone());
  return (wxThread::ExitCode)0; // success
}

void godricFrame::onThreadUpdate(wxThreadEvent& rEvent)
{
  auto f = rEvent.GetString();
  if (!f.IsEmpty()) m_pListBox->Append(f);
  int num = rEvent.GetInt();
  if (-1 == num && GetThread() && GetThread()->IsRunning()) GetThread()->Wait();
  if (m_pProgress && (-1 == num || !m_pProgress->Update(num))) endWorker();
}

void godricFrame::endWorker()
{
  m_pProgress->Destroy();
  m_pProgress = nullptr;
  if (GetThread() && GetThread()->IsRunning()) GetThread()->Wait();
  m_pInputDir->ReCreateTree();
}
