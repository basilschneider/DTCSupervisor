#include "DTCSupervisor/DTCSupervisor.h"

using namespace cgicc;

//constructor
XDAQ_INSTANTIATOR_IMPL (Ph2TkDAQ::DTCSupervisor)

Ph2TkDAQ::DTCSupervisor::DTCSupervisor (xdaq::ApplicationStub* s)
throw (xdaq::exception::Exception) : xdaq::WebApplication (s),
    fManager (this),
    fHWDescriptionFile (""),
    fXLSStylesheet (""),
    fHWFormString ("")
{
    //bind xgi and xoap commands to methods
    //methods for tab navigation
    xgi::bind (this, &DTCSupervisor::Default, "Default");
    xgi::bind (this, &DTCSupervisor::MainPage, "MainPage");
    xgi::bind (this, &DTCSupervisor::ConfigPage, "ConfigPage");
    xgi::bind (this, &DTCSupervisor::CalibrationPage, "CalibrationPage");
    xgi::bind (this, &DTCSupervisor::DAQPage, "DAQPage");

    //helper methods for buttons etc
    xgi::bind (this, &DTCSupervisor::reloadHWFile, "reloadHWFile");
    xgi::bind (this, &DTCSupervisor::handleHWFormData, "handleHWFormData");

    //make configurable variapbles available in the Application Info Space
    this->getApplicationInfoSpace()->fireItemAvailable ("HWDescriptionFile", &fHWDescriptionFile);
    this->getApplicationInfoSpace()->fireItemAvailable ("XSLStylesheet", &fXLSStylesheet);
    //detect when default values have been set
    this->getApplicationInfoSpace()->addListener (this, "urn:xdaq-event:setDefaultValues");

    //set the current view to MAIN
    fCurrentPageView = Tab::MAIN;
}

//Destructor
Ph2TkDAQ::DTCSupervisor::~DTCSupervisor() {}

//configure action listener
void Ph2TkDAQ::DTCSupervisor::actionPerformed (xdata::Event& e)
{
    if (e.type() == "urn:xdaq-event:setDefaultValues")
    {
        fHWDescriptionFile = Ph2TkDAQ::expandEnvironmentVariables (fHWDescriptionFile.toString() );
        fXLSStylesheet = Ph2TkDAQ::expandEnvironmentVariables (fXLSStylesheet.toString() );

        fHWDescriptionFile = Ph2TkDAQ::removeFilePrefix (fHWDescriptionFile.toString() );
        fXLSStylesheet = Ph2TkDAQ::removeFilePrefix (fXLSStylesheet.toString() );


        std::stringstream ss;

        ss << BLUE << "HW Description file: " << fHWDescriptionFile.toString() << " set!" << std::endl;
        ss << "XSL HW Description Stylesheet: " << fXLSStylesheet.toString() << " set!" << std::endl;
        ss << "All Default Values set!" << RESET << std::endl;
        LOG4CPLUS_INFO (this->getApplicationLogger(), ss.str() );
        //here is the listener for FSM state transition commands via xoap
        //have a look at https://gitlab.cern.ch/cms_tk_ph2/BoardSupervisor/blob/master/src/common/BoardSupervisor.cc
        //at a later point!
    }
}

void Ph2TkDAQ::DTCSupervisor::Default (xgi::Input* in, xgi::Output* out)
throw (xgi::exception::Exception)
{
    this->MainPage (in, out);
}

void Ph2TkDAQ::DTCSupervisor::MainPage (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/" + "reloadHWFile";

    // stream for logger
    std::ostringstream cLogStream;

    //define view and create header
    fCurrentPageView = Tab::MAIN;
    this->createHtmlHeader (in, out, fCurrentPageView);

    // generate the page content
    *out << cgicc::h3 ("DTCSupervisor Main Page") << std::endl;
    this->displayLoadForm (in, out);

    LOG4CPLUS_INFO (this->getApplicationLogger(), cLogStream.str() );
    this->createHtmlFooter (in, out);
}

void Ph2TkDAQ::DTCSupervisor::ConfigPage (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
    //define view and create header
    fCurrentPageView = Tab::CONFIG;
    this->createHtmlHeader (in, out, fCurrentPageView);

    //string defining action
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/" + "handleHWFormData";

    this->displayLoadForm (in, out);
    // Display the HwDescription HTML form
    *out << cgicc::form().set ("method", "POST").set ("action", url).set ("enctype", "multipart/form-data") << std::endl;
    *out << cgicc::input().set ("type", "submit").set ("title", "submit the entered values").set ("value", "Submit") << std::endl;
    *out << cgicc::input().set ("type", "reset").set ("title", "reset the form").set ("value", "Reset") << std::endl;
    *out << fHWFormString << std::endl;
    *out << cgicc::form() << std::endl;


    this->createHtmlFooter (in, out);
}

void Ph2TkDAQ::DTCSupervisor::createHtmlHeader (xgi::Input* in, xgi::Output* out, Tab pTab)
{
    // Create the Title, Tab bar
    std::ostringstream cLogStream;

    fManager.getHTMLHeader (in, out);
    //out->getHTTPResponseHeader().addHeader ("Content-Type", "text/html");
    //*out << html().set ("lang", "en").set ("dir", "ltr") << std::endl;
    //*out << HTMLDoctype (HTMLDoctype::eStrict) << std::endl;

    //*out << head() << std::endl;
    //Style this thing
    *out << style() << std::endl;
    *out << Ph2TkDAQ::parseStylesheetCSS (Ph2TkDAQ::expandEnvironmentVariables ("${DTCSUPERVISOR_ROOT}/html/Stylesheet.css"), cLogStream) << std::endl;
    *out << style() << std::endl;

    *out << title ("DTC Supervisor")  << std::endl;
    //*out << head() << std::endl;

    std::ostringstream cTabBarString;
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/";

    // switch to show the current tab
    switch (pTab)
    {
        case Tab::MAIN:
            cTabBarString << "<a href='" << url << "MainPage' class=\"button active\">MainPage</a>  <a href='" << url << "ConfigPage' class=\"button\">ConfigPage</a>  <a href='" << url << "CalibrationPage' class=\"button\">CalibrationPage</a>  <a href='" << url << "DAQPage' class=\"button\">DAQPage</a>" << std::endl;
            break;

        case Tab::CONFIG:
            cTabBarString << "<a href='" << url << "MainPage' class=\"button\">MainPage</a>  <a href='" << url << "ConfigPage' class=\"button active\">ConfigPage</a>  <a href='" << url << "CalibrationPage' class=\"button\">CalibrationPage</a>  <a href='" << url << "DAQPage' class=\"button\">DAQPage</a>" << std::endl;
            break;

        case Tab::CALIBRATION:
            cTabBarString << "<a href='" << url << "MainPage' class=\"button\">MainPage</a>  <a href='" << url << "ConfigPage' class=\"button\">ConfigPage</a>  <a href='" << url << "CalibrationPage' class=\"button active\">CalibrationPage</a>  <a href='" << url << "DAQPage' class=\"button\">DAQPage</a>" << std::endl;
            break;

        case Tab::DAQ:
            cTabBarString << "<a href='" << url << "MainPage' class=\"button\">MainPage</a>  <a href='" << url << "ConfigPage' class=\"button\">ConfigPage</a>  <a href='" << url << "CalibrationPage' class=\"button\">CalibrationPage</a>  <a href='" << url << "DAQPage' class=\"button active\">DAQPage</a>" << std::endl;
            break;
    }

    //*out << "<div class=\"title\"> <h2> DTC Supervisor </h2></div>" << std::endl;
    *out << "<div class=\"tab\">" << std::endl;
    *out << cTabBarString.str() << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"main\">" << std::endl;
    this->showStateMachineStatus (out);
    *out << "<div class=\"content\">" << std::endl;

    LOG4CPLUS_INFO (this->getApplicationLogger(), cLogStream.str() );
}

void Ph2TkDAQ::DTCSupervisor::createHtmlFooter (xgi::Input* in, xgi::Output* out)
{
    //close the main and content div
    *out << "</div class=\"content\"></div class=\"main\">" << std::endl;
    fManager.getHTMLFooter (in, out);

}

void Ph2TkDAQ::DTCSupervisor::showStateMachineStatus (xgi::Output* out) throw (xgi::exception::Exception)
{
    // create the FSM Status bar showing the current state
    try
    {
        //std::string action = toolbox::toString ("/%s/fsmTransition", getApplicationDescriptor()->getURN().c_str() );

        // display FSM
        //std::set<std::string> possibleInputs = fsm_.getInputs (fsm_.getCurrentState() );
        //std::set<std::string> allInputs = fsm_.getInputs();

        *out << "<div class=\"sidenav\">" << std::endl;
        *out << "<p class=\"state\">Current State: " << "Somestate" << "</p>" << std::endl;
        *out << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Initialize </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Configure </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Start </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Stop </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Pause </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Resume </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Halt </a>" << cgicc::br() << std::endl;
        *out << "<a href=\"#\" class=\"button\"> Destroy </a>" << cgicc::br() << std::endl;
        *out << "</div>" << std::endl;
    }
    catch (xgi::exception::Exception& e)
    {
        XCEPT_RETHROW (xgi::exception::Exception, "Exception caught in WebShowRun", e);
    }
}

void Ph2TkDAQ::DTCSupervisor::reloadHWFile (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
    // stream for logger
    std::ostringstream cLogStream;

    //parse the form input
    cgicc::Cgicc cgi (in);
    std::string cHWDescriptionFile;
    cgicc::form_iterator cIt = cgi.getElement ("HwDescriptionFile");

    if (!cIt->isEmpty() && cIt != (*cgi).end() )
    {
        cHWDescriptionFile = cIt->getValue();

        //take action
        if (!cHWDescriptionFile.empty() && Ph2TkDAQ::checkFile (cHWDescriptionFile) )
        {
            fHWDescriptionFile = cHWDescriptionFile;
            cLogStream << BLUE << "Changed HW Description File to: " << fHWDescriptionFile.toString() << RESET << std::endl;
            fHWFormString = Ph2TkDAQ::XMLUtils::transformXmlDocument (fHWDescriptionFile.toString(), fXLSStylesheet.toString(), cLogStream);
        }
        else
        {
            cLogStream << RED << "Error, HW Description File " << cHWDescriptionFile << " is an empty string or does not exist!" << RESET << std::endl;
            //*out << "<span style=\"color:red\">The selected file " << cHWDescriptionFile << " does not exist!</span>" << std::endl;
        }
    }

    LOG4CPLUS_INFO (this->getApplicationLogger(), cLogStream.str() );
    this->lastPage (in, out);
}

void Ph2TkDAQ::DTCSupervisor::handleHWFormData (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
    fHWFormVector.clear();
    std::cout << fHWFormVector.size() << std::endl;
    //stream for logger
    std::ostringstream cLogStream;

    // get the form input
    cgicc::Cgicc cgi (in);

    for (auto cIt : *cgi)
        if (cIt.getValue() != "")
        {
            //fHWFormVector.push_back (std::make_pair (Ph2TkDAQ::removeDot (cIt.getName() ), cIt.getValue() ) );
            fHWFormVector.push_back (std::make_pair (cIt.getName(), cIt.getValue() ) );
        }


    //set this to true once the HWDescription object is initialized to get a reduced set of form input pairs to modify existing HWDescription objects
    bool cStripUnchanged = true;
    Ph2TkDAQ::XMLUtils::updateHTMLForm (fHWFormString, fHWFormVector, cLogStream, cStripUnchanged );

    for (auto cPair : fHWFormVector)
        std::cout << cPair.first << " " << cPair.second << std::endl;

    LOG4CPLUS_INFO (this->getApplicationLogger(), cLogStream.str() );
    this->lastPage (in, out);
}

void Ph2TkDAQ::DTCSupervisor::displayLoadForm (xgi::Input* in, xgi::Output* out)
{
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/" + "reloadHWFile";
    *out << cgicc::div().set ("padding", "10px") << std::endl;
    *out << cgicc::form().set ("method", "POST").set ("action", url).set ("enctype", "multipart/form-data").set ("autocomplete", "on") << std::endl;
    *out << "<label for=\"HwDescriptionFile\">Hw Descritpion FilePath: </label>" << std::endl;
    //if(state==halted)
    *out << cgicc::input().set ("type", "text").set ("name", "HwDescriptionFile").set ("id", "HwDescriptionFile").set ("size", "70").set ("value", fHWDescriptionFile.toString() ) << std::endl;
    *out << cgicc::input().set ("type", "submit").set ("title", "change the Hw Description File").set ("value", "Load") << std::endl;
    //else
    //*out << cgicc::input().set ("type", "text").set ("name", "HwDescriptionFile").set ("id", "HwDescriptionFile").set ("size", "70").set ("value", fHWDescriptionFile.toString() ).set ("disabled", "disabled") << std::endl;
    //*out << cgicc::input().set ("type", "submit").set ("title", "change the Hw Description File").set ("value", "Load").set ("disabled", "disabled") << std::endl;
    *out << cgicc::form() << std::endl;
    *out << cgicc::div() << std::endl;
}
