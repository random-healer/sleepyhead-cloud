/* SleepLib Profiles Header
 *
 * Copyright (c) 2011 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#ifndef PROFILES_H
#define PROFILES_H

#include <QString>
#include <QCryptographicHash>
#include <QThread>

#include "VersionInfo.h"
#include "machine.h"
#include "machine_loader.h"
#include "preferences.h"
#include "common.h"

using namespace SleepyHead::Common;

class Machine;

enum Gender { GenderNotSpecified, Male, Female };
enum MaskType { Mask_Unknown, Mask_NasalPillows, Mask_Hybrid, Mask_StandardNasal, Mask_FullFace };
enum OverviewLinechartModes { OLC_Bartop, OLC_Lines };

class DoctorInfo;
class UserInfo;
class UserSettings;
class OxiSettings;
class CPAPSettings;
class AppearanceSettings;
class SessionSettings;

/*!
  \class Profile
  \author Mark Watkins
  \date 28/04/11
  \brief The User profile system, containing all information for a user, and an index into all Machine data
 */
class Profile : public Preferences
{
  public:
    //! \brief Constructor.. Does not open profile
    Profile(QString path);

    virtual ~Profile();

    //! \brief Open profile, parse profile.xml file, and initialize helper classes
    virtual bool Load(QString filename = "");

    //! \brief Parse machines.xml
    bool OpenMachines();
    bool StoreMachines();

    //! \brief Returns hostname that locked profile, or empty string if unlocked
    QString checkLock();

    //! \brief Removes a lockfile
    bool removeLock();

    //! \brief Save Profile object (This is an extension to Preference::Save(..))
    virtual bool Save(QString filename = "");

    //! \brief Add machine to this profiles machlist
    void AddMachine(Machine *m);

    //! \brief Remove machine from this profiles machlist
    void DelMachine(Machine *m);

    //! \brief Loads all machine (summary) data belonging to this profile
    void LoadMachineData();

    //! \brief Barf because data format has changed. This does a purge of CPAP data for machine *m
    void DataFormatError(Machine *m);

    QString path() { return p_path; }

        /*! \brief Import Machine Data
        \param path containing import location
     */
    int Import(QString path);

    //! \brief Removes a given day from the date, destroying the daylist date record if empty
    bool unlinkDay(Day * day);

//    bool trashMachine(Machine * mach);

    //! \brief Add Day record to Profile Day list
    Day *addDay(QDate date);

    //! \brief Get Day record if data available for date and machine type, else return nullptr
    Day *GetDay(QDate date, MachineType type = MT_UNKNOWN);

    //! \brief Same as GetDay but does not open the summaries
    Day *FindDay(QDate date, MachineType type = MT_UNKNOWN);

    //! \brief Get Day record if data available for date and machine type,
    //         and has enabled session data, else return nullptr
    Day *GetGoodDay(QDate date, MachineType type);

    //! \breif Same as GetGoodDay but does not open the summaries
    Day *FindGoodDay(QDate date, MachineType type);

    //! \brief Returns a list of all machines of type t
    QList<Machine *> GetMachines(MachineType t = MT_UNKNOWN);

    //! \brief Returns the machine of type t used on date, nullptr if none..
    Machine *GetMachine(MachineType t, QDate date);

    //! \brief return the first machine of type t
    Machine *GetMachine(MachineType t);

    //! \brief Returns true if this profile stores this variable identified by key
    bool contains(QString key) { return p_preferences.contains(key); }


    //! \brief Get all days records of machine type between start and end dates
    QList<Day *> getDays(MachineType mt, QDate start, QDate end);

    //! \brief Returns a count of all days (with data) of machine type, between start and end dates
    int countDays(MachineType mt = MT_UNKNOWN, QDate start = QDate(), QDate end = QDate());

    //! \brief Returns a count of all compliant days of machine type between start and end dates
    int countCompliantDays(MachineType mt, QDate start, QDate end);

    //! \brief Returns a count of all event entries for code, matching machine type between start an end dates
    EventDataType calcCount(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                            QDate end = QDate());

    //! \brief Returns a sum of all event data for Channel code, matching machine type between start an end dates
    double calcSum(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                   QDate end = QDate());

    //! \brief Returns a sum of all session durations for machine type, between start and end dates
    EventDataType calcHours(MachineType mt = MT_CPAP, QDate start = QDate(), QDate end = QDate());

    //! \brief Calculates Channel Average (Sums and counts all events, returning the sum divided by the count.)
    EventDataType calcAvg(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                          QDate end = QDate());

    //! \brief Calculates Channel Weighted Average between start and end dates
    EventDataType calcWavg(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                           QDate end = QDate());

    //! \brief Calculates the minimum value for channel code, between start and end dates
    EventDataType calcMin(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                          QDate end = QDate());

    //! \brief Calculates the maximum value for channel code, between start and end dates
    EventDataType calcMax(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                          QDate end = QDate());

    //! \brief Calculates a percentile value percent for channel code, between start and end dates
    EventDataType calcPercentile(ChannelID code, EventDataType percent, MachineType mt = MT_CPAP,
                                 QDate start = QDate(), QDate end = QDate());

    //! \brief Tests if Channel code is available in all day sets
    bool hasChannel(ChannelID code);


    //! \brief Looks up if any machines report channel is available
    bool channelAvailable(ChannelID code);


    //! \brief Calculates the minimum session settings value for channel code, between start and end dates
    EventDataType calcSettingsMin(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                                  QDate end = QDate());

    //! \brief Calculates the maximum session settings value for channel code, between start and end dates
    EventDataType calcSettingsMax(ChannelID code, MachineType mt = MT_CPAP, QDate start = QDate(),
                                  QDate end = QDate());

    //! \brief Calculates the time channel code spends above threshold value for machine type, between start and end dates
    EventDataType calcAboveThreshold(ChannelID code, EventDataType threshold, MachineType mt = MT_CPAP,
                                     QDate start = QDate(), QDate end = QDate());

    //! \brief Calculates the time channel code spends below threshold value for machine type, between start and end dates
    EventDataType calcBelowThreshold(ChannelID code, EventDataType threshold, MachineType mt = MT_CPAP,
                                     QDate start = QDate(), QDate end = QDate());


    Day * findSessionDay(Session * session);
    // XML load components
    virtual void ExtraLoad(QDomElement &root);

    //! \brief Looks for the first date containing a day record matching machinetype
    QDate FirstDay(MachineType mt = MT_UNKNOWN);

    //! \brief Looks for the last date containing a day record matching machinetype
    QDate LastDay(MachineType mt = MT_UNKNOWN);

    //! \brief Looks for the first date containing a day record with enabled sessions matching machinetype
    QDate FirstGoodDay(MachineType mt = MT_UNKNOWN);

    //! \brief Looks for the last date containing a day record with enabled sessions matching machinetype
    QDate LastGoodDay(MachineType mt = MT_UNKNOWN);

    //! \brief Returns this profiles data folder
    QString dataFolder() { return (*this).Get("{DataFolder}"); }

    //! \brief Return if this profile has been opened or not
    bool isOpen() { return m_opened; }

    //! \brief QMap of day records (iterates in order).
    QMap<QDate, Day *> daylist;

    //! \brief List of machines, indexed by MachineID.
    QHash<MachineID, Machine *> machlist;

    bool is_first_day;

    UserInfo *user;
    CPAPSettings *cpap;
    OxiSettings *oxi;
    DoctorInfo *doctor;
    AppearanceSettings *appearance;
    UserSettings *general;
    SessionSettings *session;

  protected:
    QDate m_first;
    QDate m_last;

    bool m_opened;
    bool m_machopened;
};

class MachineLoader;
extern MachineLoader *GetLoader(QString name);

extern Preferences *p_pref;
extern Preferences *p_layout;
extern Profile *p_profile;

// these are bad and must change
#define PREF (*p_pref)
#define LAYOUT (*p_layout)

// DoctorInfo Strings
const QString STR_DI_Name = "DoctorName";
const QString STR_DI_Phone = "DoctorPhone";
const QString STR_DI_Email = "DoctorEmail";
const QString STR_DI_Practice = "DoctorPractice";
const QString STR_DI_Address = "DoctorAddress";
const QString STR_DI_PatientID = "DoctorPatientID";

// UserInfo Strings
const QString STR_UI_DOB = "DOB";
const QString STR_UI_FirstName = "FirstName";
const QString STR_UI_LastName = "LastName";
const QString STR_UI_UserName = "UserName";
const QString STR_UI_Password = "Password";
const QString STR_UI_Address = "Address";
const QString STR_UI_Phone = "Phone";
const QString STR_UI_EmailAddress = "EmailAddress";
const QString STR_UI_Country = "Country";
const QString STR_UI_Height = "Height";
const QString STR_UI_Gender = "Gender";
const QString STR_UI_TimeZone = "TimeZone";
const QString STR_UI_DST = "DST";

// OxiSettings Strings
const QString STR_OS_EnableOximetry = "EnableOximetry";
const QString STR_OS_DefaultDevice = "DefaultOxiDevice";
const QString STR_OS_SyncOximeterClock = "SyncOximeterClock";
const QString STR_OS_OximeterType = "OximeterType";
const QString STR_OS_OxiDiscardThreshold = "OxiDiscardThreshold";
const QString STR_OS_SPO2DropDuration = "SPO2DropDuration";
const QString STR_OS_SPO2DropPercentage = "SPO2DropPercentage";
const QString STR_OS_PulseChangeDuration = "PulseChangeDuration";
const QString STR_OS_PulseChangeBPM = "PulseChangeBPM";
const QString STR_OS_SkipOxiIntroScreen = "SkipOxiIntroScreen";


// CPAPSettings Strings
const QString STR_CS_ComplianceHours = "ComplianceHours";
const QString STR_CS_ShowCompliance = "ShowCompliance";
const QString STR_CS_ShowLeaksMode = "ShowLeaksMode";
const QString STR_CS_MaskStartDate = "MaskStartDate";
const QString STR_CS_MaskDescription = "MaskDescription";
const QString STR_CS_MaskType = "MaskType";
const QString STR_CS_PrescribedMode = "CPAPPrescribedMode";
const QString STR_CS_PrescribedMinPressure = "CPAPPrescribedMinPressure";
const QString STR_CS_PrescribedMaxPressure = "CPAPPrescribedMaxPressure";
const QString STR_CS_UntreatedAHI = "UntreatedAHI";
const QString STR_CS_Notes = "CPAPNotes";
const QString STR_CS_DateDiagnosed = "DateDiagnosed";
const QString STR_CS_UserEventFlagging = "UserEventFlagging";
const QString STR_CS_UserEventPieChart = "UserEventPieChart";
const QString STR_CS_AutoImport = "AutoImport";
const QString STR_CS_BrickWarning = "BrickWarning";

const QString STR_CS_UserFlowRestriction = "UserFlowRestriction";
const QString STR_CS_UserEventDuration = "UserEventDuration";
const QString STR_CS_UserFlowRestriction2 = "UserFlowRestriction2";
const QString STR_CS_UserEventDuration2 = "UserEventDuration2";
const QString STR_CS_UserEventDuplicates = "UserEventDuplicates";
const QString STR_CS_ResyncFromUserFlagging = "ResyncFromUserFlagging";

const QString STR_CS_AHIWindow = "AHIWindow";
const QString STR_CS_AHIReset = "AHIReset";
const QString STR_CS_ClockDrift = "ClockDrift";
const QString STR_CS_LeakRedline = "LeakRedline";
const QString STR_CS_ShowLeakRedline = "ShowLeakRedline";

const QString STR_CS_CalculateUnintentionalLeaks = "CalculateUnintentionalLeaks";
const QString STR_CS_4cmH2OLeaks = "Custom4cmH2OLeaks";
const QString STR_CS_20cmH2OLeaks = "Custom20cmH2OLeaks";

// ImportSettings Strings
const QString STR_IS_DaySplitTime = "DaySplitTime";
const QString STR_IS_PreloadSummaries = "PreloadSummaries";
const QString STR_IS_CacheSessions = "MemoryHog";
const QString STR_IS_CombineCloseSessions = "CombineCloserSessions";
const QString STR_IS_IgnoreShorterSessions = "IgnoreShorterSessions";
const QString STR_IS_Multithreading = "EnableMultithreading";
const QString STR_IS_BackupCardData = "BackupCardData";
const QString STR_IS_CompressBackupData = "CompressBackupData";
const QString STR_IS_CompressSessionData = "CompressSessionData";
const QString STR_IS_IgnoreOlderSessions = "IgnoreOlderSessions";
const QString STR_IS_IgnoreOlderSessionsDate = "IgnoreOlderSessionsDate";
const QString STR_IS_LockSummarySessions = "LockSummarySessions";

// AppearanceSettings Strings
const QString STR_AS_GraphHeight = "GraphHeight";
const QString STR_AS_DailyPanelWidth = "DailyPanelWidth";
const QString STR_AS_RightPanelWidth = "RightPanelWidth";
const QString STR_AS_AntiAliasing = "UseAntiAliasing";
const QString STR_AS_GraphSnapshots = "EnableGraphSnapshots";
const QString STR_AS_Animations = "AnimationsAndTransitions";
const QString STR_AS_SquareWave = "SquareWavePlots";
const QString STR_AS_OverlayType = "OverlayType";
const QString STR_AS_OverviewLinechartMode = "OverviewLinechartMode";
const QString STR_AS_UsePixmapCaching = "UsePixmapCaching";
const QString STR_AS_AllowYAxisScaling = "AllowYAxisScaling";
const QString STR_AS_GraphTooltips = "GraphTooltips";
const QString STR_AS_LineThickness = "LineThickness";
const QString STR_AS_LineCursorMode = "LineCursorMode";
const QString STR_AS_CalendarVisible = "CalendarVisible";
const QString STR_AS_RightSidebarVisible = "RightSidebarVisible";

// UserSettings Strings
const QString STR_US_UnitSystem = "UnitSystem";
const QString STR_US_EventWindowSize = "EventWindowSize";
const QString STR_US_SkipEmptyDays = "SkipEmptyDays";
const QString STR_US_RebuildCache = "RebuildCache";
const QString STR_US_ShowDebug = "ShowDebug";
const QString STR_US_ShowPerformance = "ShowPerformance";
const QString STR_US_LinkGroups = "LinkGroups";
const QString STR_US_CalculateRDI = "CalculateRDI";
const QString STR_US_ShowSerialNumbers = "ShowSerialNumbers";
const QString STR_US_PrefCalcMiddle = "PrefCalcMiddle";
const QString STR_US_PrefCalcPercentile = "PrefCalcPercentile";
const QString STR_US_PrefCalcMax = "PrefCalcMax";
const QString STR_US_TooltipTimeout = "TooltipTimeout";
const QString STR_US_ScrollDampening = "ScrollDampening";
const QString STR_US_ShowUnknownFlags = "ShowUnknownFlags";
const QString STR_US_StatReportMode = "StatReportMode";
const QString STR_US_LastOverviewRange = "LastOverviewRange";

// Parent class for subclasses that manipulate the profile.
class ProfileSettings
{
  public:
    ProfileSettings(Profile *profile)
      : m_profile(profile)
    { }

    inline void setPref(QString name, QVariant value) {
        (*m_profile)[name] = value;
    }

    inline void initPref(QString name, QVariant value) {
        m_profile->init(name, value);
    }

    inline QVariant getPref(QString name) const {
        return (*m_profile)[name];
    }

    void setProfile(Profile *profile) {
        m_profile = profile;
    }

  public:
    Profile *m_profile;
};

class DoctorInfo : public ProfileSettings
{
  public:
    DoctorInfo(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_DI_Name, QString());
        initPref(STR_DI_Phone, QString());
        initPref(STR_DI_Email, QString());
        initPref(STR_DI_Practice, QString());
        initPref(STR_DI_Address, QString());
        initPref(STR_DI_PatientID, QString());
    }

    const QString name() const { return getPref(STR_DI_Name).toString(); }
    const QString phone() const { return getPref(STR_DI_Phone).toString(); }
    const QString email() const { return getPref(STR_DI_Email).toString(); }
    const QString practiceName() const { return getPref(STR_DI_Practice).toString(); }
    const QString address() const { return getPref(STR_DI_Address).toString(); }
    const QString patientID() const { return getPref(STR_DI_PatientID).toString(); }

    void setName(QString name) { setPref(STR_DI_Name, name); }
    void setPhone(QString phone) { setPref(STR_DI_Phone, phone); }
    void setEmail(QString phone) { setPref(STR_DI_Email, phone); }
    void setPracticeName(QString practice) { setPref(STR_DI_Practice, practice); }
    void setAddress(QString address) { setPref(STR_DI_Address, address); }
    void setPatientID(QString pid) { setPref(STR_DI_PatientID, pid); }
};


/*! \class UserInfo
    \brief Profile Options relating to the User Information
    */
class UserInfo : public ProfileSettings
{
  public:
    UserInfo(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_UI_DOB, QDate(1970, 1, 1));
        initPref(STR_UI_FirstName, QString());
        initPref(STR_UI_LastName, QString());
        initPref(STR_UI_UserName, QString());
        initPref(STR_UI_Password, QString());
        initPref(STR_UI_Address, QString());
        initPref(STR_UI_Phone, QString());
        initPref(STR_UI_EmailAddress, QString());
        initPref(STR_UI_Country, QString());
        initPref(STR_UI_Height, 0.0);
        initPref(STR_UI_Gender, (int)GenderNotSpecified);
        initPref(STR_UI_TimeZone, QString());
        initPref(STR_UI_DST, false);
    }

    QDate DOB() const { return getPref(STR_UI_DOB).toDate(); }
    const QString firstName() const { return getPref(STR_UI_FirstName).toString(); }
    const QString lastName() const { return getPref(STR_UI_LastName).toString(); }
    const QString userName() const { return getPref(STR_UI_UserName).toString(); }
    const QString address() const { return getPref(STR_UI_Address).toString(); }
    const QString phone() const { return getPref(STR_UI_Phone).toString(); }
    const QString email() const { return getPref(STR_UI_EmailAddress).toString(); }
    double height() const { return getPref(STR_UI_Height).toDouble(); }
    const QString country() const { return getPref(STR_UI_Country).toString(); }
    Gender gender() const { return (Gender)getPref(STR_UI_Gender).toInt(); }
    const QString timeZone() const { return getPref(STR_UI_TimeZone).toString(); }
    bool daylightSaving() const { return getPref(STR_UI_DST).toBool(); }

    void setDOB(QDate date) { setPref(STR_UI_DOB, date); }
    void setFirstName(QString name) { setPref(STR_UI_FirstName, name); }
    void setLastName(QString name) { setPref(STR_UI_LastName, name); }
    void setUserName(QString username) { setPref(STR_UI_UserName, username); }
    void setAddress(QString address) { setPref(STR_UI_Address, address); }
    void setPhone(QString phone) { setPref(STR_UI_Phone, phone); }
    void setEmail(QString email) { setPref(STR_UI_EmailAddress, email); }
    void setHeight(double height) { setPref(STR_UI_Height, height); }
    void setCountry(QString country) { setPref(STR_UI_Country, country); }
    void setGender(Gender g) { setPref(STR_UI_Gender, (int)g); }
    void setTimeZone(QString tz) { setPref(STR_UI_TimeZone, tz); }
    void setDaylightSaving(bool ds) { setPref(STR_UI_DST, ds); }

    bool hasPassword() { return !getPref(STR_UI_Password).toString().isEmpty(); }

    bool checkPassword(QString password) {
        QByteArray ba = password.toUtf8();
        QString hashedPass = QString(QCryptographicHash::hash(ba, QCryptographicHash::Sha1).toHex());
        return getPref(STR_UI_Password).toString() == hashedPass;
    }

    void setPassword(QString password) {
        QByteArray ba = password.toUtf8();
        QString hashedPass = QString(QCryptographicHash::hash(ba, QCryptographicHash::Sha1).toHex());
        setPref(STR_UI_Password, hashedPass);
    }
};

/*! \class OxiSettings
    \brief Profile Options relating to the Oximetry settings
    */
class OxiSettings : public ProfileSettings
{
  public:
    //! \brief Create OxiSettings object given Profile *p, and initialize the defaults
    OxiSettings(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_OS_EnableOximetry, false);
        initPref(STR_OS_DefaultDevice, QString());
        initPref(STR_OS_SyncOximeterClock, true);
        initPref(STR_OS_OximeterType, 0);
        initPref(STR_OS_OxiDiscardThreshold, 0.0);
        initPref(STR_OS_SPO2DropDuration, 8.0);
        initPref(STR_OS_SPO2DropPercentage, 3.0);
        initPref(STR_OS_PulseChangeDuration, 8.0);
        initPref(STR_OS_PulseChangeBPM, 5.0);
        initPref(STR_OS_SkipOxiIntroScreen, false);
    }

    bool oximetryEnabled() const { return getPref(STR_OS_EnableOximetry).toBool(); }
    QString defaultDevice() const { return getPref(STR_OS_DefaultDevice).toString(); }
    bool syncOximeterClock() const { return getPref(STR_OS_SyncOximeterClock).toBool(); }
    int oximeterType() const { return getPref(STR_OS_OximeterType).toInt(); }
    double oxiDiscardThreshold() const { return getPref(STR_OS_OxiDiscardThreshold).toDouble(); }
    double spO2DropDuration() const { return getPref(STR_OS_SPO2DropDuration).toDouble(); }
    double spO2DropPercentage() const { return getPref(STR_OS_SPO2DropPercentage).toDouble(); }
    double pulseChangeDuration() const { return getPref(STR_OS_PulseChangeDuration).toDouble(); }
    double pulseChangeBPM() const { return getPref(STR_OS_PulseChangeBPM).toDouble(); }
    bool skipOxiIntroScreen() const { return getPref(STR_OS_SkipOxiIntroScreen).toBool(); }


    void setOximetryEnabled(bool enabled) { setPref(STR_OS_EnableOximetry, enabled); }
    void setDefaultDevice(QString name) { setPref(STR_OS_DefaultDevice, name); }
    void setSyncOximeterClock(bool synced) { setPref(STR_OS_SyncOximeterClock, synced); }
    void setOximeterType(int oxitype) { setPref(STR_OS_OximeterType, oxitype); }
    void setOxiDiscardThreshold(double thresh) { setPref(STR_OS_OxiDiscardThreshold, thresh); }
    void setSpO2DropDuration(double duration) { setPref(STR_OS_SPO2DropDuration, duration); }
    void setPulseChangeBPM(double bpm) { setPref(STR_OS_PulseChangeBPM, bpm); }
    void setSkipOxiIntroScreen(bool skip) { setPref(STR_OS_SkipOxiIntroScreen, skip); }
    void setSpO2DropPercentage(double percentage) {
        setPref(STR_OS_SPO2DropPercentage, percentage);
    }
    void setPulseChangeDuration(double duration) {
        setPref(STR_OS_PulseChangeDuration, duration);
    }
};

/*! \class CPAPSettings
    \brief Profile Options relating to the CPAP settings
    */
class CPAPSettings : public ProfileSettings
{
  public:
    CPAPSettings(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_CS_ComplianceHours, 4);
        initPref(STR_CS_ShowCompliance, true);
        initPref(STR_CS_ShowLeaksMode, 0);
        // TODO: jedimark: Check if this date is initiliazed yet
        initPref(STR_CS_MaskStartDate, QDate());
        initPref(STR_CS_MaskDescription, QString());
        initPref(STR_CS_MaskType, Mask_Unknown);
        initPref(STR_CS_PrescribedMode, MODE_UNKNOWN);
        initPref(STR_CS_PrescribedMinPressure, 0.0);
        initPref(STR_CS_PrescribedMaxPressure, 0.0);
        initPref(STR_CS_UntreatedAHI, 0.0);
        initPref(STR_CS_Notes, QString());
        initPref(STR_CS_DateDiagnosed, QDate());
        initPref(STR_CS_UserFlowRestriction, 20.0);
        initPref(STR_CS_UserEventDuration, 8.0);
        initPref(STR_CS_UserFlowRestriction2, 50.0);
        initPref(STR_CS_UserEventDuration2, 8.0);
        initPref(STR_CS_UserEventDuplicates, false);
        initPref(STR_CS_UserEventFlagging, false);
        initPref(STR_CS_AHIWindow, 60.0);
        initPref(STR_CS_AHIReset, false);
        initPref(STR_CS_LeakRedline, 24.0);
        initPref(STR_CS_ShowLeakRedline, true);
        initPref(STR_CS_UserEventPieChart, false);
        initPref(STR_CS_ResyncFromUserFlagging, false);
        initPref(STR_CS_AutoImport, false);
        initPref(STR_CS_BrickWarning, true);

        initPref(STR_CS_CalculateUnintentionalLeaks, true);
        initPref(STR_CS_4cmH2OLeaks, 20.167);
        initPref(STR_CS_20cmH2OLeaks, 48.333);

        initPref(STR_CS_ClockDrift, (int)0);
        m_clock_drift = getPref(STR_CS_ClockDrift).toInt();
    }

    //Getters
    double complianceHours() const { return getPref(STR_CS_ComplianceHours).toDouble(); }
    bool showComplianceInfo() const { return getPref(STR_CS_ShowCompliance).toBool(); }
    int leakMode() const { return getPref(STR_CS_ShowLeaksMode).toInt(); }
    QDate maskStartDate() const { return getPref(STR_CS_MaskStartDate).toDate(); }
    QString maskDescription() const { return getPref(STR_CS_MaskDescription).toString(); }
    MaskType maskType() const { return (MaskType)getPref(STR_CS_MaskType).toInt(); }
    CPAPMode mode() const { return CPAPMode(getPref(STR_CS_PrescribedMode).toInt()); }
    double minPressure() const { return getPref(STR_CS_PrescribedMinPressure).toDouble(); }
    double maxPressure() const { return getPref(STR_CS_PrescribedMaxPressure).toDouble(); }
    double untreatedAHI() const { return getPref(STR_CS_UntreatedAHI).toDouble(); }
    const QString notes() const { return getPref(STR_CS_Notes).toString(); }
    QDate dateDiagnosed() const { return getPref(STR_CS_DateDiagnosed).toDate(); }
    double userFlowRestriction() const { return getPref(STR_CS_UserFlowRestriction).toDouble(); }
    double userEventDuration() const { return getPref(STR_CS_UserEventDuration).toDouble(); }
    double userFlowRestriction2() const { return getPref(STR_CS_UserFlowRestriction2).toDouble(); }
    double userEventDuration2() const { return getPref(STR_CS_UserEventDuration2).toDouble(); }
    bool userEventDuplicates() const { return getPref(STR_CS_UserEventDuplicates).toBool(); }
    double AHIWindow() const { return getPref(STR_CS_AHIWindow).toDouble(); }
    bool AHIReset() const { return getPref(STR_CS_AHIReset).toBool(); }
    bool userEventFlagging() const { return getPref(STR_CS_UserEventFlagging).toBool(); }
    int clockDrift() const { return m_clock_drift; }
    EventDataType leakRedline() const { return getPref(STR_CS_LeakRedline).toFloat(); }
    bool showLeakRedline() const { return getPref(STR_CS_ShowLeakRedline).toBool(); }
    bool userEventPieChart() const { return getPref(STR_CS_UserEventPieChart).toBool(); }
    bool resyncFromUserFlagging() const { return getPref(STR_CS_ResyncFromUserFlagging).toBool(); }
    bool autoImport() const { return getPref(STR_CS_AutoImport).toBool(); }
    bool brickWarning() const { return getPref(STR_CS_BrickWarning).toBool(); }

    bool calculateUnintentionalLeaks() const { return getPref(STR_CS_CalculateUnintentionalLeaks).toBool(); }
    double custom4cmH2OLeaks() const { return getPref(STR_CS_4cmH2OLeaks).toDouble(); }
    double custom20cmH2OLeaks() const { return getPref(STR_CS_20cmH2OLeaks).toDouble(); }

    //Setters
    void setMode(CPAPMode mode) { setPref(STR_CS_PrescribedMode, (int)mode); }
    void setMinPressure(double pressure) { setPref(STR_CS_PrescribedMinPressure, pressure); }
    void setMaxPressure(double pressure) { setPref(STR_CS_PrescribedMaxPressure, pressure); }
    void setUntreatedAHI(double ahi) { setPref(STR_CS_UntreatedAHI, ahi); }
    void setNotes(QString notes) { setPref(STR_CS_Notes, notes); }
    void setDateDiagnosed(QDate date) { setPref(STR_CS_DateDiagnosed, date); }
    void setComplianceHours(double hours) { setPref(STR_CS_ComplianceHours, hours); }
    void setShowComplianceInfo(bool b) { setPref(STR_CS_ShowCompliance, b); }
    void setLeakMode(int leakmode) { setPref(STR_CS_ShowLeaksMode, (int)leakmode); }
    void setMaskStartDate(QDate date) { setPref(STR_CS_MaskStartDate, date); }
    void setMaskType(MaskType masktype) { setPref(STR_CS_MaskType, (int)masktype); }
    void setUserFlowRestriction(double flow) { setPref(STR_CS_UserFlowRestriction, flow); }
    void setUserEventDuration(double duration) { setPref(STR_CS_UserEventDuration, duration); }
    void setUserFlowRestriction2(double flow) { setPref(STR_CS_UserFlowRestriction2, flow); }
    void setUserEventDuration2(double duration) { setPref(STR_CS_UserEventDuration2, duration); }
    void setAHIWindow(double window) { setPref(STR_CS_AHIWindow, window); }
    void setAHIReset(bool reset) { setPref(STR_CS_AHIReset, reset); }
    void setUserEventFlagging(bool flagging) { setPref(STR_CS_UserEventFlagging, flagging); }
    void setUserEventDuplicates(bool dup) { setPref(STR_CS_UserEventDuplicates, dup); }
    void setMaskDescription(QString description) {
        setPref(STR_CS_MaskDescription, description);
    }
    void setClockDrift(int seconds) {
        setPref(STR_CS_ClockDrift, m_clock_drift = (int)seconds);
    }
    void setLeakRedline(EventDataType value) { setPref(STR_CS_LeakRedline, value); }
    void setShowLeakRedline(bool reset) { setPref(STR_CS_ShowLeakRedline, reset); }
    void setUserEventPieChart(bool b) { setPref(STR_CS_UserEventPieChart, b); }
    void setResyncFromUserFlagging(bool b) { setPref(STR_CS_ResyncFromUserFlagging, b); }
    void setAutoImport(bool b) { setPref(STR_CS_AutoImport, b); }
    void setBrickWarning(bool b) { setPref(STR_CS_BrickWarning, b); }

    void setCalculateUnintentionalLeaks(bool b) { setPref(STR_CS_CalculateUnintentionalLeaks, b); }
    void setCustom4cmH2OLeaks(double val) { setPref(STR_CS_4cmH2OLeaks, val); }
    void setCustom20cmH2OLeaks(double val) { setPref(STR_CS_20cmH2OLeaks, val); }

  public:
    int m_clock_drift;
};

/*! \class ImportSettings
    \brief Profile Options relating to the Import process
    */
class SessionSettings : public ProfileSettings
{
  public:
    SessionSettings(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_IS_DaySplitTime, QTime(12, 0, 0));
        initPref(STR_IS_CacheSessions, false);
        initPref(STR_IS_PreloadSummaries, false);
        initPref(STR_IS_CombineCloseSessions, 240);
        initPref(STR_IS_IgnoreShorterSessions, 5);
        initPref(STR_IS_Multithreading, QThread::idealThreadCount() > 1);
        initPref(STR_IS_BackupCardData, true);
        initPref(STR_IS_CompressBackupData, false);
        initPref(STR_IS_CompressSessionData, false);
        initPref(STR_IS_IgnoreOlderSessions, false);
        initPref(STR_IS_IgnoreOlderSessionsDate, QDateTime(QDate::currentDate().addYears(-1), daySplitTime()) );
        initPref(STR_IS_LockSummarySessions, true);

    }

    QTime daySplitTime() const { return getPref(STR_IS_DaySplitTime).toTime(); }
    bool cacheSessions() const { return getPref(STR_IS_CacheSessions).toBool(); }
    bool preloadSummaries() const { return getPref(STR_IS_PreloadSummaries).toBool(); }
    double combineCloseSessions() const { return getPref(STR_IS_CombineCloseSessions).toDouble(); }
    double ignoreShortSessions() const { return getPref(STR_IS_IgnoreShorterSessions).toDouble(); }
    bool multithreading() const { return getPref(STR_IS_Multithreading).toBool(); }
    bool compressSessionData() const { return getPref(STR_IS_CompressSessionData).toBool(); }
    bool compressBackupData() const { return getPref(STR_IS_CompressBackupData).toBool(); }
    bool backupCardData() const { return getPref(STR_IS_BackupCardData).toBool(); }
    bool ignoreOlderSessions() const { return getPref(STR_IS_IgnoreOlderSessions).toBool(); }
    QDateTime ignoreOlderSessionsDate() const { return getPref(STR_IS_IgnoreOlderSessionsDate).toDateTime(); }
    bool lockSummarySessions() const { return getPref(STR_IS_LockSummarySessions).toBool(); }

    void setDaySplitTime(QTime time) { setPref(STR_IS_DaySplitTime, time); }
    void setCacheSessions(bool c) { setPref(STR_IS_CacheSessions, c); }
    void setPreloadSummaries(bool b) { setPref(STR_IS_PreloadSummaries, b); }
    void setCombineCloseSessions(double val) { setPref(STR_IS_CombineCloseSessions, val); }
    void setIgnoreShortSessions(double val) { setPref(STR_IS_IgnoreShorterSessions, val); }
    void setMultithreading(bool enabled) { setPref(STR_IS_Multithreading, enabled); }
    void setBackupCardData(bool enabled) { setPref(STR_IS_BackupCardData, enabled); }
    void setCompressBackupData(bool enabled) { setPref(STR_IS_CompressBackupData, enabled); }
    void setCompressSessionData(bool enabled) { setPref(STR_IS_CompressSessionData, enabled); }
    void setIgnoreOlderSessions(bool enabled) { setPref(STR_IS_IgnoreOlderSessions, enabled); }
    void setIgnoreOlderSessionsDate(QDate date) { setPref(STR_IS_IgnoreOlderSessionsDate, QDateTime(date, daySplitTime())); }
    void setLockSummarySessions(bool b) { setPref(STR_IS_LockSummarySessions, b); }

};

/*! \class AppearanceSettings
    \brief Profile Options relating to Visual Appearance
    */
class AppearanceSettings : public ProfileSettings
{
  public:
    //! \brief Create AppearanceSettings object given Profile *p, and initialize the defaults
    AppearanceSettings(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_AS_GraphHeight, 180.0);
        initPref(STR_AS_DailyPanelWidth, 350.0);
        initPref(STR_AS_RightPanelWidth, 230.0);
        initPref(STR_AS_AntiAliasing, true);
        initPref(STR_AS_GraphSnapshots, true);
        initPref(STR_AS_Animations, true);
        initPref(STR_AS_SquareWave, false);
        initPref(STR_AS_AllowYAxisScaling, true);
        initPref(STR_AS_GraphTooltips, true);
        initPref(STR_AS_UsePixmapCaching, false);
        initPref(STR_AS_OverlayType, ODT_Bars);
        initPref(STR_AS_OverviewLinechartMode, OLC_Bartop);
        initPref(STR_AS_LineThickness, 1.0);
        initPref(STR_AS_LineCursorMode, true);
        initPref(STR_AS_CalendarVisible, true);
        initPref(STR_AS_RightSidebarVisible, true);
    }

    //! \brief Returns the normal (unscaled) height of a graph
    int graphHeight() const { return getPref(STR_AS_GraphHeight).toInt(); }
    //! \brief Returns the normal (unscaled) height of a graph
    int dailyPanelWidth() const { return getPref(STR_AS_DailyPanelWidth).toInt(); }
    //! \brief Returns the normal (unscaled) height of a graph
    int rightPanelWidth() const { return getPref(STR_AS_RightPanelWidth).toInt(); }
    //! \brief Returns true if AntiAliasing (the graphical smoothing method) is enabled
    bool antiAliasing() const { return getPref(STR_AS_AntiAliasing).toBool(); }
    //! \brief Returns true if renderPixmap function is in use, which takes snapshots of graphs
    bool graphSnapshots() const { return getPref(STR_AS_GraphSnapshots).toBool(); }
    //! \brief Returns true if Graphical animations & Transitions will be drawn
    bool animations() const { return getPref(STR_AS_Animations).toBool(); }
    //! \brief Returns true if PixmapCaching acceleration will be used
    bool usePixmapCaching() const { return getPref(STR_AS_UsePixmapCaching).toBool(); }
    //! \brief Returns true if Square Wave plots are preferred (where possible)
    bool squareWavePlots() const { return getPref(STR_AS_SquareWave).toBool(); }
    //! \brief Whether to allow double clicking on Y-Axis labels to change vertical scaling mode
    bool allowYAxisScaling() const { return getPref(STR_AS_AllowYAxisScaling).toBool(); }
    //! \brief Whether to show graph tooltips
    bool graphTooltips() const { return getPref(STR_AS_GraphTooltips).toBool(); }
    //! \brief Pen width of line plots
    float lineThickness() const { return getPref(STR_AS_LineThickness).toFloat(); }
    //! \brief Whether to show line cursor
    bool lineCursorMode() const { return getPref(STR_AS_LineCursorMode).toBool(); }
    //! \brief Whether to show the calendar
    bool calendarVisible() const { return getPref(STR_AS_CalendarVisible).toBool(); }
    //! \brief Whether to show the right sidebar
    bool rightSidebarVisible() const { return getPref(STR_AS_RightSidebarVisible).toBool(); }


    //! \brief Returns the type of overlay flags (which are displayed over the Flow Waveform)
    OverlayDisplayType overlayType() const {
        return (OverlayDisplayType)getPref(STR_AS_OverlayType).toInt();
    }
    //! \brief Returns the display type of Overview pages linechart
    OverviewLinechartModes overviewLinechartMode() const {
        return (OverviewLinechartModes)getPref(STR_AS_OverviewLinechartMode).toInt();
    }

    //! \brief Set the normal (unscaled) height of a graph.
    void setGraphHeight(int height) { setPref(STR_AS_GraphHeight, height); }
    //! \brief Set the normal (unscaled) height of a graph.
    void setDailyPanelWidth(int width) { setPref(STR_AS_DailyPanelWidth, width); }
    //! \brief Set the normal (unscaled) height of a graph.
    void setRightPanelWidth(int width) { setPref(STR_AS_RightPanelWidth, width); }
    //! \brief Set to true to turn on AntiAliasing (the graphical smoothing method)
    void setAntiAliasing(bool aa) { setPref(STR_AS_AntiAliasing, aa); }
    //! \brief Set to true if renderPixmap functions are in use, which takes snapshots of graphs.
    void setGraphSnapshots(bool gs) { setPref(STR_AS_GraphSnapshots, gs); }
    //! \brief Set to true if Graphical animations & Transitions will be drawn
    void setAnimations(bool anim) { setPref(STR_AS_Animations, anim); }
    //! \brief Set to true to use Pixmap Caching of Text and other graphics caching speedup techniques
    void setUsePixmapCaching(bool b) { setPref(STR_AS_UsePixmapCaching, b); }
    //! \brief Set whether or not to useSquare Wave plots (where possible)
    void setSquareWavePlots(bool sw) { setPref(STR_AS_SquareWave, sw); }
    //! \brief Sets the type of overlay flags (which are displayed over the Flow Waveform)
    void setOverlayType(OverlayDisplayType od) { setPref(STR_AS_OverlayType, (int)od); }
    //! \brief Sets whether to allow double clicking on Y-Axis labels to change vertical scaling mode
    void setAllowYAxisScaling(bool b) { setPref(STR_AS_AllowYAxisScaling, b); }
    //! \brief Sets whether to allow double clicking on Y-Axis labels to change vertical scaling mode
    void setGraphTooltips(bool b) { setPref(STR_AS_GraphTooltips, b); }
    //! \brief Sets the type of overlay flags (which are displayed over the Flow Waveform)
    void setOverviewLinechartMode(OverviewLinechartModes od) {
        setPref(STR_AS_OverviewLinechartMode, (int)od);
    }
    //! \brief Set the pen width of line plots.
    void setLineThickness(float size) { setPref(STR_AS_LineThickness, size); }
    //! \brief Sets whether to display Line Cursor
    void setLineCursorMode(bool b) { setPref(STR_AS_LineCursorMode, b); }
    //! \brief Sets whether to display the (Daily View) Calendar
    void setCalendarVisible(bool b) { setPref(STR_AS_CalendarVisible, b); }
    //! \brief Sets whether to display the right sidebar
    void setRightSidebarVisible(bool b) { setPref(STR_AS_RightSidebarVisible, b); }
};

/*! \class UserSettings
    \brief Profile Options relating to General User Settings
    */
class UserSettings : public ProfileSettings
{
  public:
    UserSettings(Profile *profile)
      : ProfileSettings(profile)
    {
        initPref(STR_US_UnitSystem, US_Metric);
        initPref(STR_US_EventWindowSize, 4.0);
        initPref(STR_US_SkipEmptyDays, true);
        initPref(STR_US_RebuildCache, false); // FIXME: jedimark: can't remember...
        initPref(STR_US_ShowDebug, false);
        initPref(STR_US_ShowPerformance, false);
        initPref(STR_US_CalculateRDI, false);
        initPref(STR_US_ShowSerialNumbers, false);
        initPref(STR_US_PrefCalcMiddle, (int)0);
        initPref(STR_US_PrefCalcPercentile, (double)95.0);
        initPref(STR_US_PrefCalcMax, (int)0);
        initPref(STR_US_TooltipTimeout, (int)2500);
        initPref(STR_US_ScrollDampening, (int)50);
        initPref(STR_US_StatReportMode, 0);
        initPref(STR_US_ShowUnknownFlags, false);
        initPref(STR_US_LastOverviewRange, 4);
    }

    UnitSystem unitSystem() const { return (UnitSystem)getPref(STR_US_UnitSystem).toInt(); }
    double eventWindowSize() const { return getPref(STR_US_EventWindowSize).toDouble(); }
    bool skipEmptyDays() const { return getPref(STR_US_SkipEmptyDays).toBool(); }
    bool rebuildCache() const { return getPref(STR_US_RebuildCache).toBool(); }
    bool showDebug() const { return getPref(STR_US_ShowDebug).toBool(); }
    bool showPerformance() const { return getPref(STR_US_ShowPerformance).toBool(); }
    bool calculateRDI() const { return getPref(STR_US_CalculateRDI).toBool(); }
    bool showSerialNumbers() const { return getPref(STR_US_ShowSerialNumbers).toBool(); }
    int prefCalcMiddle() const { return getPref(STR_US_PrefCalcMiddle).toInt(); }
    double prefCalcPercentile() const { return getPref(STR_US_PrefCalcPercentile).toDouble(); }
    int prefCalcMax() const { return getPref(STR_US_PrefCalcMax).toInt(); }
    int tooltipTimeout() const { return getPref(STR_US_TooltipTimeout).toInt(); }
    int scrollDampening() const { return getPref(STR_US_ScrollDampening).toInt(); }
    int statReportMode() const { return getPref(STR_US_StatReportMode).toInt(); }
    bool showUnknownFlags() const { return getPref(STR_US_ShowUnknownFlags).toBool(); }
    int lastOverviewRange() const { return getPref(STR_US_LastOverviewRange).toInt(); }

    void setUnitSystem(UnitSystem us) { setPref(STR_US_UnitSystem, (int)us); }
    void setEventWindowSize(double size) { setPref(STR_US_EventWindowSize, size); }
    void setSkipEmptyDays(bool skip) { setPref(STR_US_SkipEmptyDays, skip); }
    void setRebuildCache(bool rebuild) { setPref(STR_US_RebuildCache, rebuild); }
    void setShowDebug(bool b) { setPref(STR_US_ShowDebug, b); }
    void setShowPerformance(bool b) { setPref(STR_US_ShowPerformance, b); }
    void setCalculateRDI(bool rdi) { setPref(STR_US_CalculateRDI, rdi); }
    void setShowSerialNumbers(bool enabled) { setPref(STR_US_ShowSerialNumbers, enabled); }
    void setPrefCalcMiddle(int i) { setPref(STR_US_PrefCalcMiddle, i); }
    void setPrefCalcPercentile(double p) { setPref(STR_US_PrefCalcPercentile, p); }
    void setPrefCalcMax(int i) { setPref(STR_US_PrefCalcMax, i); }
    void setTooltipTimeout(int i) { setPref(STR_US_TooltipTimeout, i); }
    void setScrollDampening(int i) { setPref(STR_US_ScrollDampening, i); }
    void setStatReportMode(int i) { setPref(STR_US_StatReportMode, i); }
    void setShowUnknownFlags(bool b) { setPref(STR_US_ShowUnknownFlags, b); }
    void setLastOverviewRange(int i) { setPref(STR_US_LastOverviewRange, i); }
};

//! \brief Returns a count of all files & directories in a supplied folder
int dirCount(QString path);


namespace Profiles {

extern QMap<QString, Profile *> profiles;
void Scan(); // Initialize and load Profile
void Done(); // Save all Profile objects and clear list

Profile *Create(QString name);
Profile *Get(QString name);
Profile *Get();

}

#endif // PROFILES_H

