/* SleepLib PRS1 Loader Implementation
 *
 * Copyright (c) 2011-2016 Mark Watkins <jedimark@users.sourceforge.net>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of the Linux
 * distribution for more details. */

#include <QApplication>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QProgressBar>
#include <QDebug>
#include <cmath>
#include "SleepLib/schema.h"
#include "prs1_loader.h"
#include "SleepLib/session.h"
#include "SleepLib/calcs.h"


//const int PRS1_MAGIC_NUMBER = 2;
//const int PRS1_SUMMARY_FILE=1;
//const int PRS1_EVENT_FILE=2;
//const int PRS1_WAVEFORM_FILE=5;


//********************************************************************************************
/// IMPORTANT!!!
//********************************************************************************************
// Please INCREMENT the prs1_data_version in prs1_loader.h when making changes to this loader
// that change loader behaviour or modify channels.
//********************************************************************************************


extern QProgressBar *qprogress;

QHash<int, QString> ModelMap;

#define PRS1_CRC_CHECK

#ifdef PRS1_CRC_CHECK
typedef quint16 crc_t;

static const crc_t crc_table[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

crc_t CRC16(const unsigned char *data, size_t data_len)
{
    crc_t crc = 0;
    unsigned int tbl_idx;

    while (data_len--) {
        tbl_idx = (crc ^ *data) & 0xff;
        crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffff;

        data++;
    }

    return crc & 0xffff;
}
#endif

enum FlexMode { FLEX_None, FLEX_CFlex, FLEX_CFlexPlus, FLEX_AFlex, FLEX_RiseTime, FLEX_BiFlex, FLEX_Unknown  };

ChannelID PRS1_TimedBreath = 0, PRS1_HeatedTubing = 0;

PRS1::PRS1(MachineID id): CPAP(id)
{
}
PRS1::~PRS1()
{

}


/*! \struct WaveHeaderList
    \brief Used in PRS1 Waveform Parsing */
struct WaveHeaderList {
    quint16 interleave;
    quint8  sample_format;
    WaveHeaderList(quint16 i, quint8 f) { interleave = i; sample_format = f; }
};


PRS1Loader::PRS1Loader()
{
    const QString PRS1_ICON = ":/icons/prs1.png";
    const QString PRS1_60_ICON = ":/icons/prs1_60s.png";
    const QString DREAMSTATION_ICON = ":/icons/dreamstation.png";

   // QString s = newInfo().series;
    m_pixmap_paths["System One"] = PRS1_ICON;
    m_pixmaps["System One"] = QPixmap(PRS1_ICON);
    m_pixmap_paths["System One (60 Series)"] = PRS1_60_ICON;
    m_pixmaps["System One (60 Series)"] = QPixmap(PRS1_60_ICON);
    m_pixmap_paths["DreamStation"] = DREAMSTATION_ICON;
    m_pixmaps["DreamStation"] = QPixmap(DREAMSTATION_ICON);

    //genCRCTable();  // find what I did with this..
    m_type = MT_CPAP;
}

PRS1Loader::~PRS1Loader()
{
}

bool isdigit(QChar c)
{
    if ((c >= '0') && (c <= '9')) { return true; }

    return false;
}

const QString PR_STR_PSeries = "P-Series";


// Tests path to see if it has (what looks like) a valid PRS1 folder structure
bool PRS1Loader::Detect(const QString & path)
{
    QString newpath = checkDir(path);

    return !newpath.isEmpty();
}


QString PRS1Loader::checkDir(const QString & path)
{
    QString newpath = path;

    newpath.replace("\\", "/");

    if (!newpath.endsWith("/" + PR_STR_PSeries)) {
        newpath = path + "/" + PR_STR_PSeries;
    }

    QDir dir(newpath);

    if ((!dir.exists() || !dir.isReadable())) {
        return QString();
    }
    qDebug() << "PRS1Loader::Detect path=" << newpath;

    QFile lastfile(newpath+"/last.txt");

    bool exists = true;
    if (!lastfile.exists()) {
        lastfile.setFileName(newpath+"/LAST.TXT");
        if (!lastfile.exists())
            exists = false;
    }

    QString machpath;
    if (exists) {
        if (!lastfile.open(QIODevice::ReadOnly)) {
            qDebug() << "PRS1Loader: last.txt exists but I couldn't open it!";
        } else {
            QTextStream ts(&lastfile);
            QString serial = ts.readLine(64).trimmed();
            lastfile.close();

            machpath = newpath+"/"+serial;

            if (!QDir(machpath).exists()) {
                machpath = QString();
            }
        }
    }

    if (machpath.isEmpty()) {
        QDir dir(newpath);
        QStringList dirs = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
        if (dirs.size() > 0) {
            machpath = dir.cleanPath(newpath+"/"+dirs[0]);

        }
    }


    return machpath;
}

void parseModel(MachineInfo & info, QString modelnum)
{
    info.modelnumber = modelnum;

    QString modelstr;
    bool fnd = false;
    for (int i=0; i<modelnum.size(); i++) {
        QChar c = modelnum.at(i);
        if (c.isDigit()) {
            modelstr += c;
            fnd = true;
        } else if (fnd) break;
    }

    bool ok;
    int num = modelstr.toInt(&ok);

    int series = ((num / 10) % 10);
    int type = (num / 100);
    int country = num % 10;


    switch (type) {
    case 1: // cpap
    case 2: // cpap
    case 3: // cpap
        info.model = QObject::tr("RemStar Plus Compliance Only");
        break;
    case 4: // cpap
        info.model = QObject::tr("RemStar Pro with C-Flex+");
        break;
    case 5: // apap
        info.model = QObject::tr("RemStar Auto with A-Flex");
        break;
    case 6: // bipap
        info.model = QObject::tr("RemStar BiPAP Pro with Bi-Flex");
        break;
    case 7: // bipap auto
        info.model = QObject::tr("RemStar BiPAP Auto with Bi-Flex");
        break;
    case 9: // asv
        info.model = QObject::tr("BiPAP autoSV Advanced");
        break;
    case 10: // Avaps
        info.model = QObject::tr("BiPAP AVAPS");
        break;
    default:
        info.model = QObject::tr("Unknown Model");
    }

    switch (series) {
    case 5:
        info.series = QObject::tr("System One");
        break;
    case 6:
        info.series = QObject::tr("System One (60 Series)");
        break;
    case 7:
        info.series = QObject::tr("DreamStation");
        break;
    default:
        info.series = QObject::tr("unknown");
        break;

    }
    switch (country) {
    case '0':
        break;
    case '1':
        break;
    default:
        break;
    }
}

bool PRS1Loader::PeekProperties(MachineInfo & info, QString filename, Machine * mach)
{
    QFile f(filename);
    if (!f.open(QFile::ReadOnly)) {
        return false;
    }
    QTextStream in(&f);
    QString modelnum;
    int ptype=0;
    int dfv=0;
    bool ok;
    do {
        QString line = in.readLine();
        QStringList pair = line.split("=");

        bool skip = false;

        if (pair[0].contains("ModelNumber", Qt::CaseInsensitive) || pair[0].contains("MN", Qt::CaseInsensitive)) {
            modelnum = pair[1];
            skip = true;
        }
        if (pair[0].contains("SerialNumber", Qt::CaseInsensitive) || pair[0].contains("SN", Qt::CaseInsensitive)) {
            info.serial = pair[1];
            skip = true;
        }
        if (pair[0].contains("ProductType", Qt::CaseInsensitive) || pair[0].contains("PT", Qt::CaseInsensitive)) {
            ptype = pair[1].toInt(&ok, 16);
            skip = true;
        }
        if (pair[0].contains("DataFormatVersion", Qt::CaseInsensitive) || pair[0].contains("DFV", Qt::CaseInsensitive)) {
            dfv = pair[1].toInt(&ok, 10);
            skip = true;
        }
        if (!mach || skip) continue;

        mach->properties[pair[0]] = pair[1];

    } while (!in.atEnd());

    if (!modelnum.isEmpty()) {
        parseModel(info, modelnum);
    }

    if (ptype > 0) {
        if (ModelMap.contains(ptype)) {
            info.model = ModelMap[ptype];
        }
    }

    if (dfv == 3) {
        info.series = QObject::tr("DreamStation");
    }


    return true;
}


MachineInfo PRS1Loader::PeekInfo(const QString & path)
{
    QString newpath = checkDir(path);
    if (newpath.isEmpty())
        return MachineInfo();

    MachineInfo info = newInfo();
    info.serial = newpath.section("/", -1);

    if (!PeekProperties(info, newpath+"/properties.txt")) {
        PeekProperties(info, newpath+"/PROP.TXT");
    }
    return info;
}


int PRS1Loader::Open(QString path)
{
    QString newpath;
    path = path.replace("\\", "/");

    if (path.endsWith("/" + PR_STR_PSeries)) {
        newpath = path;
    } else {
        newpath = path + "/" + PR_STR_PSeries;
    }

    qDebug() << "PRS1Loader::Open path=" << newpath;

    QDir dir(newpath);

    if ((!dir.exists() || !dir.isReadable())) {
        return -1;
    }

    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList flist = dir.entryInfoList();

    QStringList SerialNumbers;
    QStringList::iterator sn;

    for (int i = 0; i < flist.size(); i++) {
        QFileInfo fi = flist.at(i);
        QString filename = fi.fileName();

        if (fi.isDir() && (filename.size() > 4) && (isdigit(filename[1])) && (isdigit(filename[2]))) {
            SerialNumbers.push_back(filename);
        } else if (filename.toLower() == "last.txt") { // last.txt points to the current serial number
            QString file = fi.canonicalFilePath();
            QFile f(file);

            if (!fi.isReadable()) {
                qDebug() << "PRS1Loader: last.txt exists but I couldn't read it!";
                continue;
            }

            if (!f.open(QIODevice::ReadOnly)) {
                qDebug() << "PRS1Loader: last.txt exists but I couldn't open it!";
                continue;
            }

            last = f.readLine(64);
            last = last.trimmed();
            f.close();
        }
    }

    if (SerialNumbers.empty()) { return -1; }

    int c = 0;

    for (sn = SerialNumbers.begin(); sn != SerialNumbers.end(); sn++) {
        if ((*sn)[0].isLetter()) {
            c += OpenMachine(newpath + "/" + *sn);
        }
    }
    for (sn = SerialNumbers.begin(); sn != SerialNumbers.end(); sn++) {
        if (!(*sn)[0].isLetter()) {
            c += OpenMachine(newpath + "/" + *sn);
        }
    }

    return c;
}

/*bool PRS1Loader::ParseProperties(Machine *m, QString filename)
{
    QFile f(filename);

    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    QString line;
    QHash<QString, QString> prop;

    QString s = f.readLine();
    QChar sep = '=';
    QString key, value;

    MachineInfo info = newInfo();
    bool ok;

    while (!f.atEnd()) {
        key = s.section(sep, 0, 0);

        if (key == s) { continue; }

        value = s.section(sep, 1).trimmed();

        if (value == s) { continue; }

        if (key.contains("serialnumber",Qt::CaseInsensitive)) {
            info.serial = value;
        } else if (key.contains("modelnumber",Qt::CaseInsensitive)) {
            parseModel(info, value);
        } else {
            if (key.contains("producttype", Qt::CaseInsensitive)) {
                int i = value.toInt(&ok, 16);

                if (ok) {
                    if (ModelMap.find(i) != ModelMap.end()) {
                        info.model = ModelMap[i];
                    }
                }
            }
            prop[key] = value;
        }
        s = f.readLine();
    }

    if (info.serial != m->serial()) {
        qDebug() << "Serial Number in PRS1 properties.txt doesn't match machine record";
    }
    m->setInfo(info);

    for (QHash<QString, QString>::iterator i = prop.begin(); i != prop.end(); i++) {
        m->properties[i.key()] = i.value();
    }

    f.close();
    return true;
}*/

int PRS1Loader::OpenMachine(QString path)
{
    Q_ASSERT(p_profile != nullptr);

    qDebug() << "Opening PRS1 " << path;
    QDir dir(path);

    if (!dir.exists() || (!dir.isReadable())) {
        return 0;
    }

    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QFileInfoList flist = dir.entryInfoList();

    QString filename;

    if (qprogress) { qprogress->setValue(0); }

    QStringList paths;

    int sessionid_base = 10;

    QString propertyfile;

    for (int i = 0; i < flist.size(); i++) {
        QFileInfo fi = flist.at(i);
        filename = fi.fileName();

        if (fi.isDir()) {
            if ((filename[0].toLower() == 'p') && (isdigit(filename[1]))) {
                // p0, p1, p2.. etc.. folders contain the session data
                paths.push_back(fi.canonicalFilePath());
            } else if (filename.toLower() == "e") {
                // Error files..
                // Reminder: I have been given some info about these. should check it over.
            }
        } else if (filename.compare("properties.txt",Qt::CaseInsensitive) == 0) {
            propertyfile = fi.canonicalFilePath();
        } else if (filename.compare("PROP.TXT",Qt::CaseInsensitive) == 0) {
            sessionid_base = 16;
            propertyfile = fi.canonicalFilePath();
        }
    }

    MachineInfo info = newInfo();
    // Have a peek first to get the serial number.
    PeekProperties(info, propertyfile);

    QString modelstr;
    bool fnd = false;
    for (int i=0; i<info.modelnumber.size(); i++) {
        QChar c = info.modelnumber.at(i);
        if (c.isDigit()) {
            modelstr += c;
            fnd = true;
        } else if (fnd) break;
    }

    bool ok;
    int model = modelstr.toInt(&ok);
    if (ok) {
        int series = ((model / 10) % 10);
        int type = (model / 100);

        // Assumption is made here all PRS1 machines less than 450P are not data capable.. this could be wrong one day.
        if ((type < 4) && p_profile->cpap->brickWarning()) {
            QApplication::processEvents();
            QMessageBox::information(QApplication::activeWindow(),
                                     QObject::tr("Non Data Capable Machine"),
                                     QString(QObject::tr("Your Philips Respironics CPAP machine (Model %1) is unfortunately not a data capable model.")+"\n\n"+
                                             QObject::tr("I'm sorry to report that SleepyHead can only track hours of use and very basic settings for this machine.")).
                                     arg(info.modelnumber),QMessageBox::Ok);
            p_profile->cpap->setBrickWarning(false);

        }

        // A bit of protection against future annoyances..
        if (((series != 5) && (series != 6) && (series != 0))) { // || (type >= 10)) {
            QMessageBox::information(QApplication::activeWindow(),
                                     QObject::tr("Machine Unsupported"),
                                     QObject::tr("Sorry, your Philips Respironics CPAP machine (Model %1) is not supported yet.").arg(info.modelnumber) +"\n\n"+
                                     QObject::tr("JediMark needs a .zip copy of this machines' SD card and matching Encore .pdf reports to make it work with SleepyHead.")
                                     ,QMessageBox::Ok);

            return -1;
        }
    } else {
        // model number didn't parse.. Meh... Silently ignore it
//        QMessageBox::information(QApplication::activeWindow(),
//                                 QObject::tr("Machine Unsupported"),
//                                 QObject::tr("SleepyHead could not parse the model number, this machine can not be imported..") +"\n\n"+
//                                 QObject::tr("JediMark needs a .zip copy of this machines' SD card and matching Encore .pdf reports to make it work with SleepyHead.")
//                                ,QMessageBox::Ok);
        return -1;
    }


    // Which is needed to get the right machine record..
    Machine *m = CreateMachine(info);

    // This time supply the machine object so it can populate machine properties..
    PeekProperties(m->info, propertyfile, m);

    QString backupPath = m->getBackupPath() + path.section("/", -2);

    if (QDir::cleanPath(path).compare(QDir::cleanPath(backupPath)) != 0) {
        copyPath(path, backupPath);
    }


    SessionID sid;
    long ext;

    int size = paths.size();

    sesstasks.clear();
    new_sessions.clear(); // this hash is used by OpenFile


    PRS1Import * task = nullptr;
    // Note, I have observed p0/p1/etc folders containing duplicates session files (in Robin Sanders data.)

    // for each p0/p1/p2/etc... folder
    for (int p=0; p < size; ++p) {
        dir.setPath(paths.at(p));

        if (!dir.exists() || !dir.isReadable()) { continue; }

        flist = dir.entryInfoList();

        // Scan for individual session files
        for (int i = 0; i < flist.size(); i++) {
            QFileInfo fi = flist.at(i);

            QString ext_s = fi.fileName().section(".", -1);
            ext = ext_s.toInt(&ok);
            if (!ok) {
                // not a numerical extension
                continue;
            }

            QString session_s = fi.fileName().section(".", 0, -2);
            sid = session_s.toInt(&ok, sessionid_base);
            if (!ok) {
                // not a numerical session ID
                continue;
            }

            if (m->SessionExists(sid)) {
                // Skip already imported session
                continue;
            }

            if ((ext == 5) || (ext == 6)) {
                // Waveform files aren't grouped... so we just want to add the filename for later
                QHash<SessionID, PRS1Import *>::iterator it = sesstasks.find(sid);
                if (it != sesstasks.end()) {
                    task = it.value();
                } else {
                    // Should probably check if session already imported has this data missing..

                    // Create the group if we see it first..
                    task = new PRS1Import(this, sid, m);
                    sesstasks[sid] = task;
                    queTask(task);
                }

                if (ext == 5) {
                    if (!task->wavefile.isEmpty()) continue;
                    task->wavefile = fi.canonicalFilePath();
                } else if (ext == 6) {
                    if (!task->oxifile.isEmpty()) continue;
                    task->oxifile = fi.canonicalFilePath();
                }

                continue;
            }

            // Parse the data chunks and read the files..
            QList<PRS1DataChunk *> Chunks = ParseFile(fi.canonicalFilePath());

            for (int i=0; i < Chunks.size(); ++i) {
                PRS1DataChunk * chunk = Chunks.at(i);

                if (ext <= 1) {
                    const unsigned char * data = (unsigned char *)chunk->m_data.constData();

                    if (data[0x00] != 0) {
                        delete chunk;
                        continue;
                    }
                }

                SessionID chunk_sid = chunk->sessionid;
                if (m->SessionExists(sid)) {
                    delete chunk;
                    continue;
                }


                task = nullptr;
                QHash<SessionID, PRS1Import *>::iterator it = sesstasks.find(chunk_sid);
                if (it != sesstasks.end()) {
                    task = it.value();
                } else {
                    task = new PRS1Import(this, chunk_sid, m);
                    sesstasks[chunk_sid] = task;
                    // save a loop an que this now
                    queTask(task);
                }
                switch (ext) {
                case 0:
                    if (task->compliance) continue;
                    task->compliance = chunk;
                    break;
                case 1:
                    if (task->summary) continue;
                    task->summary = chunk;
                    break;
                case 2:
                    if (task->event) continue;
                    task->event = chunk;
                    break;
                default:
                    break;
                }
            }
        }
    }


    int tasks = countTasks();
    runTasks(p_profile->session->multithreading());
    finishAddingSessions();

    return m->unsupported() ? -1 : tasks;
}

bool PRS1Import::ParseF5Events()
{
    ChannelID Codes[] = {
        PRS1_00, PRS1_01, CPAP_Pressure, CPAP_EPAP, CPAP_PressurePulse, CPAP_Obstructive,
        CPAP_ClearAirway, CPAP_Hypopnea, PRS1_08,  CPAP_FlowLimit, PRS1_0A, CPAP_PB,
        PRS1_0C, CPAP_VSnore, PRS1_0E, PRS1_0F,
        CPAP_LargeLeak, // Large leak apparently
        CPAP_LeakTotal, PRS1_12
    };

    int ncodes = sizeof(Codes) / sizeof(ChannelID);
    EventList *Code[0x20] = {nullptr};

    EventList *OA = session->AddEventList(CPAP_Obstructive, EVL_Event);
    EventList *HY = session->AddEventList(CPAP_Hypopnea, EVL_Event);

    EventList *PB = session->AddEventList(CPAP_PB, EVL_Event);
    EventList *TOTLEAK = session->AddEventList(CPAP_LeakTotal, EVL_Event);
    EventList *LEAK = session->AddEventList(CPAP_Leak, EVL_Event);
    EventList *LL = session->AddEventList(CPAP_LargeLeak, EVL_Event);
    EventList *SNORE = session->AddEventList(CPAP_Snore, EVL_Event);
    EventList *IPAP = session->AddEventList(CPAP_IPAP, EVL_Event, 0.1F);
    EventList *EPAP = session->AddEventList(CPAP_EPAP, EVL_Event, 0.1F);
    EventList *PS = session->AddEventList(CPAP_PS, EVL_Event, 0.1F);
    EventList *IPAPLo = session->AddEventList(CPAP_IPAPLo, EVL_Event, 0.1F);
    EventList *IPAPHi = session->AddEventList(CPAP_IPAPHi, EVL_Event, 0.1F);
    EventList *RR = session->AddEventList(CPAP_RespRate, EVL_Event);
    EventList *PTB = session->AddEventList(CPAP_PTB, EVL_Event);
    EventList *TB = session->AddEventList(PRS1_TimedBreath, EVL_Event);

    EventList *MV = session->AddEventList(CPAP_MinuteVent, EVL_Event);
    EventList *TV = session->AddEventList(CPAP_TidalVolume, EVL_Event, 10.0F);


    EventList *CA = session->AddEventList(CPAP_ClearAirway, EVL_Event);
    EventList *FL = session->AddEventList(CPAP_FlowLimit, EVL_Event);
    EventList *VS = session->AddEventList(CPAP_VSnore, EVL_Event);
  //  EventList *VS2 = session->AddEventList(CPAP_VSnore2, EVL_Event);

    //EventList * PRESSURE=nullptr;
    //EventList * PP=nullptr;

    EventDataType data0, data1, data2, data4, data5;

    EventDataType currentPressure=0, leak; //, p;

    bool calcLeaks = p_profile->cpap->calculateUnintentionalLeaks();
    float lpm4 = p_profile->cpap->custom4cmH2OLeaks();
    float lpm20 = p_profile->cpap->custom20cmH2OLeaks();

    float lpm = lpm20 - lpm4;
    float ppm = lpm / 16.0;


    //qint64 start=timestamp;
    qint64 t = qint64(event->timestamp) * 1000L;
    session->updateFirst(t);
    qint64 tt;
    int pos = 0;
    int cnt = 0;
    short delta;//,duration;
    bool badcode = false;
    unsigned char lastcode3 = 0, lastcode2 = 0, lastcode = 0, code = 0;
    int lastpos = 0, startpos = 0, lastpos2 = 0, lastpos3 = 0;

    int size = event->m_data.size();
    unsigned char * buffer = (unsigned char *)event->m_data.data();

    while (pos < size) {
        lastcode3 = lastcode2;
        lastcode2 = lastcode;
        lastcode = code;
        lastpos3 = lastpos2;
        lastpos2 = lastpos;
        lastpos = startpos;
        startpos = pos;
        code = buffer[pos++];

        if (code >= ncodes) {
            qDebug() << "Illegal PRS1 code " << hex << int(code) << " appeared at " << hex << startpos << "in" << event->sessionid;;
            qDebug() << "1: (" << int(lastcode) << hex << lastpos << ")";
            qDebug() << "2: (" << int(lastcode2) << hex << lastpos2 << ")";
            qDebug() << "3: (" << int(lastcode3) << hex << lastpos3 << ")";
            return false;
        }

        if (code == 0) {
        } else if (code != 0x12) {
            delta = buffer[pos];
            //duration=buffer[pos+1];
            //delta=buffer[pos+1] << 8 | buffer[pos];
            pos += 2;
            t += qint64(delta) * 1000L;
        }

        ChannelID cpapcode = Codes[(int)code];
        //EventDataType PS;
        tt = t;
        cnt++;
        int fc = 0;

        switch (code) {
        case 0x00: // Unknown (ASV Pressure value)
            // offset?
            data0 = buffer[pos++];
            fc++;

            if (!buffer[pos - 1]) { // WTH???
                data1 = buffer[pos++];
                fc++;
            }

            if (!buffer[pos - 1]) {
                data2 = buffer[pos++];
                fc++;
            }

            break;

        case 0x01: // Unknown
            if (!Code[1]) {
                if (!(Code[1] = session->AddEventList(cpapcode, EVL_Event, 0.1F))) { return false; }
            }

            Code[1]->AddEvent(t, 0);
            break;

        case 0x02: // Pressure ???
            data0 = buffer[pos++];
            //            if (!Code[2]) {
            //                if (!(Code[2]=session->AddEventList(cpapcode,EVL_Event,0.1))) return false;
            //            }
            //            Code[2]->AddEvent(t,data0);
            break;
        case 0x03: // BIPAP Pressure
            qDebug() << "0x03 Observed in ASV data!!????";

            data0 = buffer[pos++];
            data1 = buffer[pos++];
            //            data0/=10.0;
            //            data1/=10.0;
            //            session->AddEvent(new Event(t,CPAP_EAP, 0, data, 1));
            //            session->AddEvent(new Event(t,CPAP_IAP, 0, &data1, 1));
            break;

        case 0x04: // Timed Breath
            data0 = buffer[pos++];

            TB->AddEvent(t, data0);
            break;

        case 0x05:
            //code=CPAP_Obstructive;
            data0 = buffer[pos++];
            tt -= qint64(data0) * 1000L; // Subtract Time Offset
            OA->AddEvent(tt, data0);
            break;

        case 0x06:
            //code=CPAP_ClearAirway;
            data0 = buffer[pos++];
            tt -= qint64(data0) * 1000L; // Subtract Time Offset

            CA->AddEvent(tt, data0);
            break;

        case 0x07:
            //code=CPAP_Hypopnea;
            data0 = buffer[pos++];
            tt -= qint64(data0) * 1000L; // Subtract Time Offset
            HY->AddEvent(tt, data0);
            break;

        case 0x08: // ???
            data0 = buffer[pos++];
            tt -= qint64(data0) * 1000L; // Subtract Time Offset
            //qDebug() << "Code 8 found at " << hex << pos - 1 << " " << tt;

            if (event->familyVersion>=2) {
                HY->AddEvent(tt, data0);
            } else {
                if (!Code[10]) {
                    if (!(Code[10] = session->AddEventList(cpapcode, EVL_Event))) { return false; }
                }

                //????
                //data1=buffer[pos++]; // ???
                Code[10]->AddEvent(tt, data0);
                //pos++;
            }
            break;

        case 0x09: // ASV Codes
            if (event->familyVersion<2) {
                //code=CPAP_FlowLimit;
                data0 = buffer[pos++];
                tt -= qint64(data0) * 1000L; // Subtract Time Offset

                FL->AddEvent(tt, data0);
            } else {
                data0 = buffer[pos++];
                data1 = buffer[pos++];
            }

            break;

        case 0x0a:
            data0 = buffer[pos++];
            tt -= qint64(data0) * 1000L; // Subtract Time Offset
            if (event->familyVersion>=2) {
                FL->AddEvent(tt, data0);
            } else {
                if (!Code[7]) {
                    if (!(Code[7] = session->AddEventList(cpapcode, EVL_Event))) { return false; }
                }

                Code[7]->AddEvent(tt, data0);
            }
            break;


        case 0x0b: // Cheyne Stokes
            data0 = ((unsigned char *)buffer)[pos + 1] << 8 | ((unsigned char *)buffer)[pos];
            //data0*=2;
            pos += 2;
            data1 = ((unsigned char *)buffer)[pos]; //|buffer[pos+1] << 8
            pos += 1;
            //tt-=delta;
            tt -= qint64(data1) * 1000L;

            if (!PB) {
                if (!(PB = session->AddEventList(cpapcode, EVL_Event))) {
                    qDebug() << "!PB addeventlist exit";
                    return false;
                }
            }

            PB->AddEvent(tt, data0);
            break;

        case 0x0c:

            if (event->familyVersion>=2) {
                data0 = (buffer[pos + 1] << 8 | buffer[pos]);
                data0 *= 2;
                pos += 2;
                data1 = buffer[pos++];
                tt = t - qint64(data1) * 1000L;

                if (!PB) {
                    if (!(PB = session->AddEventList(cpapcode, EVL_Event))) {
                        qDebug() << "!PB addeventlist exit";
                        return false;
                    }
                }

                PB->AddEvent(tt, data0);

            } else {
                data0 = buffer[pos++];
                tt -= qint64(data0) * 1000L; // Subtract Time Offset
                qDebug() << "Code 12 found at " << hex << pos - 1 << " " << tt;


                if (!Code[8]) {
                    if (!(Code[8] = session->AddEventList(cpapcode, EVL_Event))) { return false; }
                }

                Code[8]->AddEvent(tt, data0);
                pos += 2;
            }
            break;

        case 0x0d: // All the other ASV graph stuff.

            if (event->familyVersion>=2) {
                data0 = (buffer[pos + 1] << 8 | buffer[pos]);
                data0 *= 2;
                pos += 2;
                data1 = buffer[pos++];
                tt = t - qint64(data1) * 1000L;
            } else {
                IPAP->AddEvent(t, currentPressure = data0 = buffer[pos++]); // 00=IAP
                data4 = buffer[pos++];
                IPAPLo->AddEvent(t, data4);               // 01=IAP Low
                data5 = buffer[pos++];
                IPAPHi->AddEvent(t, data5);               // 02=IAP High

                TOTLEAK->AddEvent(t, leak=buffer[pos++]);           // 03=LEAK
                if (calcLeaks) { // Much Quicker doing this here than the recalc method.
                    leak -= (((currentPressure/10.0f) - 4.0) * ppm + lpm4);
                    if (leak < 0) leak = 0;

                    LEAK->AddEvent(t, leak);
                }


                RR->AddEvent(t, buffer[pos++]);             // 04=Breaths Per Minute
                PTB->AddEvent(t, buffer[pos++]);            // 05=Patient Triggered Breaths
                MV->AddEvent(t, buffer[pos++]);             // 06=Minute Ventilation
                //tmp=buffer[pos++] * 10.0;
                TV->AddEvent(t, buffer[pos++]);             // 07=Tidal Volume
                SNORE->AddEvent(t, data2 = buffer[pos++]); // 08=Snore

                if (data2 > 0) {
                    if (!VS) {
                        if (!(VS = session->AddEventList(CPAP_VSnore, EVL_Event))) {
                            qDebug() << "!VS eventlist exit";
                            return false;
                        }
                    }

                    VS->AddEvent(t, 0); //data2); // VSnore
                }

                EPAP->AddEvent(t, data1 = buffer[pos++]); // 09=EPAP
                data2 = data0 - data1;
                PS->AddEvent(t, data2);           // Pressure Support
                if (event->familyVersion >= 1) {
                    data0 = buffer[pos++];
                }
            }
            break;

        case 0x0e: // Unknown
            // Family 5.2 has this code
            if (event->familyVersion>=2) {
                EPAP->AddEvent(t, data0=buffer[pos+9]); // 9
                IPAP->AddEvent(t, data1=buffer[pos+0]); // 0
                IPAPLo->AddEvent(t, buffer[pos+1]); // 1
                IPAPHi->AddEvent(t, buffer[pos+2]); // 2
                LEAK->AddEvent(t, buffer[pos+3]); // 3
                TV->AddEvent(t, buffer[pos+7]); // 7
                RR->AddEvent(t, buffer[pos+4]); // 4
                PTB->AddEvent(t, buffer[pos+5]);  // 5
                MV->AddEvent(t,  buffer[pos+6]); //6
                SNORE->AddEvent(t, data2 = buffer[pos+8]); //??

                if (data2 > 0) {
                    if (!VS) {
                        if (!(VS = session->AddEventList(CPAP_VSnore, EVL_Event))) {
                            qDebug() << "!VS eventlist exit";
                            return false;
                        }
                    }

                    VS->AddEvent(t, 0); //data2); // VSnore
                }
                data2 = data1 - data0;
                PS->AddEvent(t, data2);           // Pressure Support
                pos+=11;
            } else {
                qDebug() << "0x0E Observed in ASV data!!????";
                data0 = buffer[pos++]; // << 8) | buffer[pos];

            }
            //session->AddEvent(new Event(t,cpapcode, 0, data, 1));
            break;
        case 0x0f:
            qDebug() << "0x0f Observed in ASV data!!????";

            data0 = buffer[pos + 1] << 8 | buffer[pos];
            pos += 2;
            data1 = buffer[pos]; //|buffer[pos+1] << 8
            pos += 1;
            tt -= qint64(data1) * 1000L;
            //session->AddEvent(new Event(tt,cpapcode, 0, data, 2));
            break;

        case 0x10: // Unknown
            data0 = buffer[pos + 1] << 8 | buffer[pos];
            pos += 2;
            data1 = buffer[pos++];

            tt = t - qint64(data1) * 1000L;
            LL->AddEvent(tt, data0);

//            qDebug() << "0x10 Observed in ASV data!!????";
//            data0 = buffer[pos++]; // << 8) | buffer[pos];
//            data1 = buffer[pos++];
//            data2 = buffer[pos++];
            //session->AddEvent(new Event(t,cpapcode, 0, data, 3));
            break;
        case 0x11: // Not Leak Rate
            qDebug() << "0x11 Observed in ASV data!!????";
            //if (!Code[24]) {
            //   Code[24]=new EventList(cpapcode,EVL_Event);
            //}
            //Code[24]->AddEvent(t,buffer[pos++]);
            break;


        case 0x12: // Summary
            qDebug() << "0x12 Observed in ASV data!!????";
            data0 = buffer[pos++];
            data1 = buffer[pos++];
            data2 = buffer[pos + 1] << 8 | buffer[pos];
            pos += 2;
            //session->AddEvent(new Event(t,cpapcode, 0, data,3));
            break;

        default:  // ERROR!!!
            qWarning() << "Some new fandangled PRS1 code detected " << hex << int(code) << " at " << pos - 1;
            badcode = true;
            break;
        }

        if (badcode) {
            break;
        }
    }

    session->updateLast(t);
    session->m_cnt.clear();
    session->m_cph.clear();

//    EventDataType minEpap = session->Min(CPAP_EPAP);
//    EventDataType minIpapLo = session->Min(CPAP_IPAPLo);
//    EventDataType maxIpapHi = session->Max(CPAP_IPAPHi);

//    session->settings[CPAP_IPAPLo] = minIpapLo;
//    session->settings[CPAP_IPAPHi] = maxIpapHi;

//    session->settings[CPAP_PSMax] = maxIpapHi - minEpap;
//    session->settings[CPAP_PSMin] = minIpapLo - minEpap;

    session->m_valuesummary[CPAP_Pressure].clear();
    session->m_valuesummary.erase(session->m_valuesummary.find(CPAP_Pressure));

    return true;

}

bool PRS1Import::ParseF3Events()
{
    qint64 t = qint64(event->timestamp) * 1000L, tt;

    session->updateFirst(t);
    EventList *OA = session->AddEventList(CPAP_Obstructive, EVL_Event);
    EventList *HY = session->AddEventList(CPAP_Hypopnea, EVL_Event);
    EventList *CA = session->AddEventList(CPAP_ClearAirway, EVL_Event);
    EventList *LEAK = session->AddEventList(CPAP_LeakTotal, EVL_Event);
    EventList *ULK = session->AddEventList(CPAP_Leak, EVL_Event);
    EventList *MV = session->AddEventList(CPAP_MinuteVent, EVL_Event);
    //EventList *TMV = session->AddEventList(CPAP_TgMV, EVL_Event);
    EventList *TV = session->AddEventList(CPAP_TidalVolume, EVL_Event,10.0);
    EventList *RR = session->AddEventList(CPAP_RespRate, EVL_Event);
    EventList *PTB = session->AddEventList(CPAP_PTB, EVL_Event);
    EventList *EPAP = session->AddEventList(CPAP_EPAP, EVL_Event,0.1);
    EventList *IPAP = session->AddEventList(CPAP_IPAP, EVL_Event,0.1);
    EventList *FLOW = session->AddEventList(CPAP_FlowRate, EVL_Event);

    int size = event->m_data.size()/0x10;
    unsigned char * h = (unsigned char *)event->m_data.data();

    int hy, oa, ca;
    qint64 div = 0;

    const qint64 block_duration = 120000;

    for (int x=0; x < size; x++) {
        IPAP->AddEvent(t, h[0] | (h[1] << 8));
        EPAP->AddEvent(t, h[2] | (h[3] << 8));
        LEAK->AddEvent(t, h[4]);
        TV->AddEvent(t, h[5]);
        FLOW->AddEvent(t, h[6]);
        PTB->AddEvent(t, h[7]);
        RR->AddEvent(t, h[8]);
        //TMV->AddEvent(t, h[9]); // not sure what this is.. encore doesn't graph it.
        MV->AddEvent(t, h[11]);
        ULK->AddEvent(t, h[15]);

        hy = h[12];  // count of hypopnea events
        ca = h[13];  // count of clear airway events
        oa = h[14];  // count of obstructive events

        // divide each event evenly over the 2 minute block
        if (hy > 0) {
            div = block_duration / hy;

            tt = t;
            for (int i=0; i < hy; ++i) {
                HY->AddEvent(t, hy);
                tt += div;
            }
        }
        if (ca > 0) {
            div = block_duration / ca;

            tt = t;

            for (int i=0; i < ca; ++i) {
                CA->AddEvent(tt, ca);
                tt += div;
            }
        }
        if (oa > 0) {
            div = block_duration / oa;

            tt = t;
            for (int i=0; i < oa; ++i) {
                OA->AddEvent(t, oa);
                tt += div;
            }
        }

        h += 0x10;
        t += block_duration;
    }
    return true;
}

extern float CatmullRomSpline(float p0, float p1, float p2, float p3, float t = 0.5);

void SmoothEventList(Session * session, EventList * ev, ChannelID code)
{
    if (!ev) return;
    int cnt = ev->count();
    if (cnt > 4) {
        EventList * smooth = new EventList(EVL_Event, ev->gain());

        smooth->setFirst(ev->first());
        smooth->AddEvent(ev->time(0), ev->raw(0));

        float p0, p1, p2, p3, v;
        for (int i=1; i<cnt-2; ++i) {
            qint64 time = ev->time(i);
            qint64 time2 = ev->time(i+1);
            qint64 diff = time2 - time;

            // these aren't evenly spaced... spline won't work here.
            p0 = ev->raw(i-1);
            p1 = ev->raw(i);
            p2 = ev->raw(i+1);
            p3 = ev->raw(i+2);

            smooth->AddEvent(time, p1);

//            int df = p2-p1;
//            if (df > 0) {
//                qint64 inter = diff/(df+1);
//                qint64 t = time+inter;
//                for (int j=0; j<df; ++j) {
//                    smooth->AddEvent(t, p1+j);
//                    t+=inter;
//                }
//            } else if (df<0) {
//                df = abs(df);
//                qint64 inter = diff/(df+1);
//                qint64 t = time+inter;
//                for (int j=0; j<df; ++j) {
//                    smooth->AddEvent(t, p1-j);
//                    t+=inter;
//                }
//            }
            // don't want to use Catmull here...


            v = CatmullRomSpline(p0, p1, p2, p3, 0.25);
            smooth->AddEvent(time+diff*0.25, v);
            v = CatmullRomSpline(p0, p1, p2, p3, 0.5);
            smooth->AddEvent(time+diff*0.5, v);
            v = CatmullRomSpline(p0, p1, p2, p3, 0.75);
            smooth->AddEvent(time+diff*0.75, v);

        }
        smooth->AddEvent(ev->time(cnt-2), ev->raw(cnt-2));
        smooth->AddEvent(ev->time(cnt-1), ev->raw(cnt-1));


        session->eventlist[code].removeAll(ev);
        delete ev;
        session->eventlist[code].append(smooth);
    }

}

bool PRS1Import::ParseF0Events()
{
    unsigned char code=0;
    EventList *Code[0x20] = {0};

    EventDataType data0, data1, data2;
    int cnt = 0;
    short delta;
    int pos;
    qint64 t = qint64(event->timestamp) * 1000L, tt;

    session->updateFirst(t);

    EventList *OA = session->AddEventList(CPAP_Obstructive, EVL_Event);
    EventList *HY = session->AddEventList(CPAP_Hypopnea, EVL_Event);
    EventList *PB = session->AddEventList(CPAP_PB, EVL_Event);
    EventList *TOTLEAK = session->AddEventList(CPAP_LeakTotal, EVL_Event);
    EventList *LEAK = session->AddEventList(CPAP_Leak, EVL_Event);
    EventList *SNORE = session->AddEventList(CPAP_Snore, EVL_Event);

    EventList *PP = session->AddEventList(CPAP_PressurePulse, EVL_Event);
    EventList *RE = session->AddEventList(CPAP_RERA, EVL_Event);
    EventList *CA = session->AddEventList(CPAP_ClearAirway, EVL_Event);
    EventList *FL = session->AddEventList(CPAP_FlowLimit, EVL_Event);
    EventList *VS = session->AddEventList(CPAP_VSnore, EVL_Event);
    EventList *VS2 = session->AddEventList(CPAP_VSnore2, EVL_Event);
    //EventList *T1 = session->AddEventList(CPAP_Test1, EVL_Event, 0.1);

    Code[17] = session->AddEventList(PRS1_0E, EVL_Event);
    EventList * LL = session->AddEventList(CPAP_LargeLeak, EVL_Event);

    EventList *PRESSURE = nullptr;
    EventList *EPAP = nullptr;
    EventList *IPAP = nullptr;
    EventList *PS = nullptr;

    unsigned char lastcode3 = 0, lastcode2 = 0, lastcode = 0;
    int lastpos = 0, startpos = 0, lastpos2 = 0, lastpos3 = 0;

    int size = event->m_data.size();

    bool FV3 = (event->fileVersion == 3);
    unsigned char * buffer = (unsigned char *)event->m_data.data();

    EventDataType currentPressure=0, leak; //, p;

    bool calcLeaks = p_profile->cpap->calculateUnintentionalLeaks();
    float lpm4 = p_profile->cpap->custom4cmH2OLeaks();
    float lpm20 = p_profile->cpap->custom20cmH2OLeaks();

    float lpm = lpm20 - lpm4;
    float ppm = lpm / 16.0;

    CPAPMode mode = (CPAPMode) session->settings[CPAP_Mode].toInt();

    for (pos = 0; pos < size;) {
        lastcode3 = lastcode2;
        lastcode2 = lastcode;
        lastcode = code;
        lastpos3 = lastpos2;
        lastpos2 = lastpos;
        lastpos = startpos;
        startpos = pos;
        code = buffer[pos++];

        if (code > 0x15) {
            qDebug() << "Illegal PRS1 code " << hex << int(code) << " appeared at " << hex << startpos << "in" << event->sessionid;
            qDebug() << "1: (" << hex << int(lastcode) << hex << lastpos << ")";
            qDebug() << "2: (" << hex << int(lastcode2) << hex << lastpos2 << ")";
            qDebug() << "3: (" << hex << int(lastcode3) << hex << lastpos3 << ")";
            return false;
        }

        if (code != 0x12) {
            delta = buffer[pos + 1] << 8 | buffer[pos];
            pos += 2;

            t += qint64(delta) * 1000L;
            tt = t;
        }

        cnt++;

        switch (code) {

        case 0x00: // Unknown 00

            if (!Code[0]) {
                if (!(Code[0] = session->AddEventList(PRS1_00, EVL_Event))) { return false; }
            }

            Code[0]->AddEvent(t, buffer[pos++]);

            if (((event->family == 0) && (event->familyVersion >= 4)) || (event->fileVersion == 3)){
                pos++;
            }

            break;

        case 0x01: // Unknown
            if ((event->family == 0) && (event->familyVersion >= 4)) {
                if (!PRESSURE) {
                    PRESSURE = session->AddEventList(CPAP_Pressure, EVL_Event, 0.1F);

                    if (!PRESSURE) { return false; }
                }

                PRESSURE->AddEvent(t, currentPressure = buffer[pos++]);
            } else {
                if (!Code[1]) {
                    if (!(Code[1] = session->AddEventList(PRS1_01, EVL_Event))) { return false; }
                }

                Code[1]->AddEvent(t, 0);
            }

            break;

        case 0x02: // Pressure
            if ((event->family == 0) && (event->familyVersion >= 4)) {  // BiPAP Pressure
                if (!EPAP) {
                    if (!(EPAP = session->AddEventList(CPAP_EPAP, EVL_Event, 0.1F))) { return false; }

                    if (!(IPAP = session->AddEventList(CPAP_IPAP, EVL_Event, 0.1F))) { return false; }

                    if (!(PS = session->AddEventList(CPAP_PS, EVL_Event, 0.1F))) { return false; }
                }

                EPAP->AddEvent(t, data0 = buffer[pos++]);
                IPAP->AddEvent(t, data1 = currentPressure = buffer[pos++]);
                PS->AddEvent(t, data1 - data0);
            } else {
                if (!PRESSURE) {
                    PRESSURE = session->AddEventList(CPAP_Pressure, EVL_Event, 0.1F);

                    if (!PRESSURE) { return false; }
                }

                PRESSURE->AddEvent(t, currentPressure = buffer[pos++]);
            }

            break;

        case 0x03: // BIPAP Pressure
            if (FV3) {
                if (!PRESSURE) {
                    PRESSURE = session->AddEventList(CPAP_Pressure, EVL_Event, 0.1F);

                    if (!PRESSURE) { return false; }
                }
                PRESSURE->AddEvent(t, currentPressure = buffer[pos++]);

            } else {
                if (!EPAP) {
                    if (!(EPAP = session->AddEventList(CPAP_EPAP, EVL_Event, 0.1F))) { return false; }

                    if (!(IPAP = session->AddEventList(CPAP_IPAP, EVL_Event, 0.1F))) { return false; }

                    if (!(PS = session->AddEventList(CPAP_PS, EVL_Event, 0.1F))) { return false; }
                }

                EPAP->AddEvent(t, data0 = buffer[pos++]);
                IPAP->AddEvent(t, data1 = currentPressure = buffer[pos++]);
                PS->AddEvent(t, data1 - data0);
            }
            break;

        case 0x04: // Pressure Pulse
            data0 = buffer[pos++];

            PP->AddEvent(t, data0);
            break;

        case 0x05: // RERA
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);

            RE->AddEvent(tt, data0);
            break;

        case 0x06: // Obstructive Apoanea
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);
            OA->AddEvent(tt, data0);
            break;

        case 0x07: // Clear Airway
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);

            CA->AddEvent(tt, data0);
            break;

        case 0x0a: // Hypopnea
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);
            HY->AddEvent(tt, data0);
            break;

        case 0x0c: // Flow Limitation
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);

            FL->AddEvent(tt, data0);
            break;

        case 0x0b: // Breathing not Detected flag???? but it doesn't line up
            data0 = buffer[pos];
            data1 = buffer[pos+1];
            pos += 2;

            if (event->familyVersion >= 4) {
                 // might not doublerize on older machines?
              //  data0 *= 2;
            }
//            data1 = buffer[pos++];

            //tt = t - qint64((data0+data1)*2) * 1000L;

            if (!Code[12]) {
                Code[12] = session->AddEventList(PRS1_0B, EVL_Event);
            }

            // FIXME
            Code[12]->AddEvent(t, data0);
            break;

        case 0x0d: // Vibratory Snore
            VS->AddEvent(t, 0);
            break;

        case 0x0e: // Unknown
            data0 = buffer[pos + 1] << 8 | buffer[pos];
            if (event->familyVersion >= 4) {
                 // might not doublerize on older machines?
                data0 *= 2;
            }

            pos += 2;
            data1 = buffer[pos++];

            tt = t - qint64(data1) * 1000L;
            Code[17]->AddEvent(tt, data0);

            break;

        case 0x0f: // Cheyne Stokes Respiration
            data0 = (buffer[pos + 1] << 8 | buffer[pos]);
            if (event->familyVersion >= 4) {
                 // might not doublerize on older machines
                data0 *= 2;
            }
            pos += 2;
            data1 = buffer[pos++];
            tt = t - qint64(data1) * 1000L;
            PB->AddEvent(tt, data0);
            break;

        case 0x10: // Large Leak
            data0 = buffer[pos + 1] << 8 | buffer[pos];
            if (event->familyVersion >= 4) {
                 // might not doublerize on older machines
                data0 *= 2;
            }
            pos += 2;
            data1 = buffer[pos++];

            tt = t - qint64(data1) * 1000L;
            LL->AddEvent(tt, data0);
            break;

        case 0x11: // Leak Rate & Snore Graphs
            data0 = buffer[pos++];
            data1 = buffer[pos++];

            TOTLEAK->AddEvent(t, data0);
            SNORE->AddEvent(t, data1);

            if (calcLeaks) { // Much Quicker doing this here than the recalc method.
                leak = data0-(((currentPressure/10.0f) - 4.0) * ppm + lpm4);
                if (leak < 0) leak = 0;

                LEAK->AddEvent(t, leak);
            }

            if (data1 > 0) {
                VS2->AddEvent(t, data1);
            }

            if ((event->family == 0) && (event->familyVersion >= 4))  {
                // EPAP / Flex Pressure
                data0 = buffer[pos++];

                // Perhaps this check is not necessary, as it will theoretically add extra resolution to pressure chart
                // for bipap models and above???
                if (mode <= MODE_BILEVEL_FIXED) {
                    if (!EPAP) {
                        if (!(EPAP = session->AddEventList(CPAP_EPAP, EVL_Event, 0.1F))) { return false; }
                    }
                    EPAP->AddEvent(t, data0);
                }
            }

            break;

        case 0x12: // Summary
            data0 = buffer[pos++];
            data1 = buffer[pos++];
            data2 = buffer[pos + 1] << 8 | buffer[pos];
            pos += 2;

            // Could end here, but I've seen data sets valid data after!!!

            break;

        case 0x14:  // DreamStation Hypopnea
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);
            HY->AddEvent(tt, data0);
            break;

        case 0x15:  // DreamStation Hypopnea
            data0 = buffer[pos++];
            tt = t - (qint64(data0) * 1000L);
            HY->AddEvent(tt, data0);
            break;

        default:
            // ERROR!!!
            qWarning() << "Some new fandangled PRS1 code detected in" << event->sessionid << hex
                       << int(code) << " at " << pos - 1;
            return false;
        }
    }

//    SmoothEventList(session, PRESSURE, CPAP_Pressure);
//    SmoothEventList(session, IPAP, CPAP_IPAP);
//    SmoothEventList(session, EPAP, CPAP_EPAP);


    session->updateLast(t);
    session->m_cnt.clear();
    session->m_cph.clear();
    session->m_lastchan.clear();
    session->m_firstchan.clear();
    session->m_valuesummary[CPAP_Pressure].clear();
    session->m_valuesummary.erase(session->m_valuesummary.find(CPAP_Pressure));

    return true;
}


bool PRS1Import::ParseCompliance()
{
    const unsigned char * data = (unsigned char *)compliance->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->settings[CPAP_Mode] = (int)MODE_CPAP;

    EventDataType min_pressure = float(data[0x03]) / 10.0;
   // EventDataType max_pressure = float(data[0x04]) / 10.0;

    session->settings[CPAP_Pressure] = min_pressure;


    int ramp_time = data[0x06];
    EventDataType ramp_pressure = float(data[0x07]) / 10.0;

    session->settings[CPAP_RampTime] = (int)ramp_time;
    session->settings[CPAP_RampPressure] = ramp_pressure;


    quint8 flex = data[0x09];
    int flexlevel = flex & 0x03;


    FlexMode flexmode = FLEX_Unknown;

    flex &= 0xf8;
    //bool split = false;

    if (flex & 0x40) {  // This bit defines the Flex setting for the CPAP component of the Split night
      //  split = true;
    }
    if (flex & 0x80) { // CFlex bit
        if (flex & 8) { // Plus bit
            flexmode = FLEX_CFlexPlus;
        } else {
            flexmode = FLEX_CFlex;
        }
    } else flexmode = FLEX_None;

    session->settings[PRS1_FlexMode] = (int)flexmode;
    session->settings[PRS1_FlexLevel] = (int)flexlevel;
    session->setSummaryOnly(true);
    //session->settings[CPAP_SummaryOnly] = true;

    session->settings[PRS1_HumidStatus] = (bool)(data[0x0A] & 0x80);        // Humidifier Connected
    session->settings[PRS1_HumidLevel] = (int)(data[0x0A] & 7);          // Humidifier Value

    // need to parse a repeating structure here containing lengths of mask on/off..
    // 0x03 = mask on
    // 0x01 = mask off

    qint64 start = qint64(compliance->timestamp) * 1000L;
    qint64 tt = start;

    int len = compliance->size()-3;
    int pos = 0x11;
    do {
        quint8 c = data[pos++];
        quint64 duration = data[pos] | data[pos+1] << 8;
        pos+=2;
        duration *= 1000L;
        SliceStatus status;
        if (c == 0x03) {
            status = EquipmentOn;
        } else if (c == 0x02) {
            status = EquipmentLeaking;
        } else if (c == 0x01) {
            status = EquipmentOff;
        } else {
            qDebug() << compliance->sessionid << "Wasn't expecting" << c;
            break;
        }
        session->m_slices.append(SessionSlice(tt, tt + duration, status));
        qDebug() << compliance->sessionid << "Added Slice" << tt << (tt+duration) << status;

        tt += duration;
    } while (pos < len);

    session->set_first(start);
    session->set_last(tt);

    // Bleh!! There is probably 10 different formats for these useless piece of junk machines
    return true;
}

bool PRS1Import::ParseSummaryF0()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    CPAPMode cpapmode = MODE_UNKNOWN;

    switch (data[0x02]) {  // PRS1 mode   // 0 = CPAP, 2 = APAP
    case 0x00:
        cpapmode = MODE_CPAP;
        break;
    case 0x01:
        cpapmode = MODE_BILEVEL_FIXED;
        break;
    case 0x02:
        cpapmode = MODE_APAP;
        break;
    case 0x03:
        cpapmode = MODE_BILEVEL_AUTO_VARIABLE_PS;
    }

    EventDataType min_pressure = float(data[0x03]) / 10.0;
    EventDataType max_pressure = float(data[0x04]) / 10.0;
    EventDataType ps  = float(data[0x05]) / 10.0; // pressure support

    if (cpapmode == MODE_CPAP) {
        session->settings[CPAP_Pressure] = min_pressure;
    } else if (cpapmode == MODE_APAP) {
        session->settings[CPAP_PressureMin] = min_pressure;
        session->settings[CPAP_PressureMax] = max_pressure;
    } else if (cpapmode == MODE_BILEVEL_FIXED) {
        session->settings[CPAP_EPAP] = min_pressure;
        session->settings[CPAP_IPAP] = max_pressure;
        session->settings[CPAP_PS] = ps;
    } else if (cpapmode == MODE_BILEVEL_AUTO_VARIABLE_PS) {
        session->settings[CPAP_EPAPLo] = min_pressure;
        session->settings[CPAP_EPAPHi] = max_pressure - 2.0;
        session->settings[CPAP_IPAPLo] = min_pressure + 2.0;
        session->settings[CPAP_IPAPHi] = max_pressure;
        session->settings[CPAP_PSMin] = 2.0f;
        session->settings[CPAP_PSMax] = ps;
    }

    session->settings[CPAP_Mode] = (int)cpapmode;


    int ramp_time = data[0x06];
    EventDataType ramp_pressure = float(data[0x07]) / 10.0;

    session->settings[CPAP_RampTime] = (int)ramp_time;
    session->settings[CPAP_RampPressure] = ramp_pressure;

    // Tubing lock has no setting byte

    // Menu Options
    session->settings[PRS1_SysLock] = (bool) (data[0x0a] & 0x80); // System One Resistance Lock Setting
    session->settings[PRS1_SysOneResistSet] = (int)data[0x0a] & 7;       // SYstem One Resistance setting value
    session->settings[PRS1_SysOneResistStat] = (bool) (data[0x0a] & 0x40);  // System One Resistance Status bit
    session->settings[PRS1_HoseDiam] = (data[0x0a] & 0x08) ? QObject::tr("15mm") : QObject::tr("22mm");
    session->settings[PRS1_AutoOn] = (bool) (data[0x0b] & 0x40);
    session->settings[PRS1_AutoOff] = (bool) (data[0x0c] & 0x10);
    session->settings[PRS1_MaskAlert] = (bool) (data[0x0c] & 0x08);
    session->settings[PRS1_ShowAHI] = (bool) (data[0x0c] & 0x04);
    session->settings[PRS1_HumidStatus] = (bool)(data[0x09] & 0x80);        // Humidifier Connected
    session->settings[PRS1_HumidLevel] = (int)(data[0x09] & 7);          // Humidifier Value

   // session->

    quint8 flex = data[0x08];

    int flexlevel = flex & 0x03;
    FlexMode flexmode = FLEX_Unknown;

    // 88 CFlex+ / AFlex (depending on CPAP mode)
    // 80 CFlex
    // 00 NoFlex
    // c0 Split CFlex then None
    // c8 Split CFlex+ then None

    flex &= 0xf8;
    bool split = false;

    if (flex & 0x40) {  // This bit defines the Flex setting for the CPAP component of the Split night
        split = true;
    }
    if (flex & 0x80) { // CFlex bit
        if (flex & 0x10) {
            flexmode = FLEX_RiseTime;
        } else if (flex & 8) { // Plus bit
            if (split || (cpapmode == MODE_CPAP)) {
                flexmode = FLEX_CFlexPlus;
            } else if (cpapmode == MODE_APAP) {
                flexmode = FLEX_AFlex;
            }
        } else {
            // CFlex bits refer to Rise Time on BiLevel machines
            flexmode = (cpapmode >= MODE_BILEVEL_FIXED) ? FLEX_BiFlex : FLEX_CFlex;
        }
    } else flexmode = FLEX_None;

    session->settings[PRS1_FlexMode] = (int)flexmode;
    session->settings[PRS1_FlexLevel] = (int)flexlevel;

    summary_duration = data[0x14] | data[0x15] << 8;

    return true;
}

bool PRS1Import::ParseSummaryF0V4()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    CPAPMode cpapmode = MODE_UNKNOWN;

    switch (data[0x02]) {  // PRS1 mode   // 0 = CPAP, 2 = APAP
    case 0x00:
        cpapmode = MODE_CPAP;
        break;
    case 0x20:
        cpapmode = MODE_BILEVEL_FIXED;
        break;
    case 0x40:
        cpapmode = MODE_APAP;
        break;
    case 0x60:
        cpapmode = MODE_BILEVEL_AUTO_VARIABLE_PS;
    }


    EventDataType min_pressure = float(data[0x03]) / 10.0;
    EventDataType max_pressure = float(data[0x04]) / 10.0;
    EventDataType min_ps  = float(data[0x05]) / 10.0; // pressure support
    EventDataType max_ps  = float(data[0x06]) / 10.0; // pressure support

    if (cpapmode == MODE_CPAP) {
        session->settings[CPAP_Pressure] = min_pressure;
    } else if (cpapmode == MODE_APAP) {
        session->settings[CPAP_PressureMin] = min_pressure;
        session->settings[CPAP_PressureMax] = max_pressure;
    } else if (cpapmode == MODE_BILEVEL_FIXED) {
        session->settings[CPAP_EPAP] = min_pressure;
        session->settings[CPAP_IPAP] = max_pressure;
        session->settings[CPAP_PS] = max_pressure - min_pressure;
    } else if (cpapmode == MODE_BILEVEL_AUTO_VARIABLE_PS) {
        session->settings[CPAP_EPAPLo] = min_pressure;
        session->settings[CPAP_EPAPHi] = max_pressure;
        session->settings[CPAP_IPAPLo] = min_pressure + min_ps;
        session->settings[CPAP_IPAPHi] = max_pressure;
        session->settings[CPAP_PSMin] = min_ps;
        session->settings[CPAP_PSMax] = max_ps;
    }
    session->settings[CPAP_Mode] = (int)cpapmode;

    quint8 flex = data[0x0a];

    int flexlevel = flex & 0x03;
    FlexMode flexmode = FLEX_Unknown;

    flex &= 0xf8;
    bool split = false;

    if (flex & 0x40) {  // This bit defines the Flex setting for the CPAP component of the Split night
        split = true;
    }
    if (flex & 0x80) { // CFlex bit
        if (flex & 0x10) {
            flexmode = FLEX_RiseTime;
        } else if (flex & 8) { // Plus bit
            if (split || (cpapmode == MODE_CPAP)) {
                flexmode = FLEX_CFlexPlus;
            } else if (cpapmode == MODE_APAP) {
                flexmode = FLEX_AFlex;
            }
        } else {
            // CFlex bits refer to Rise Time on BiLevel machines
            flexmode = (cpapmode >= MODE_BILEVEL_FIXED) ? FLEX_BiFlex : FLEX_CFlex;
        }
    } else flexmode = FLEX_None;

    int ramp_time = data[0x08];
    EventDataType ramp_pressure = float(data[0x09]) / 10.0;

    session->settings[CPAP_RampTime] = (int)ramp_time;
    session->settings[CPAP_RampPressure] = ramp_pressure;

    session->settings[PRS1_FlexMode] = (int)flexmode;
    session->settings[PRS1_FlexLevel] = (int)flexlevel;

    session->settings[PRS1_HumidStatus] = (bool)(data[0x0b] & 0x80);        // Humidifier Connected
    session->settings[PRS1_HeatedTubing] = (bool)(data[0x0b] & 0x10);        // Heated Hose??
    session->settings[PRS1_HumidLevel] = (int)(data[0x0b] & 7);          // Humidifier Value


    summary_duration = data[0x14] | data[0x15] << 8;

    return true;
}


bool PRS1Import::ParseSummaryF3()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);
    summary_duration = data[0x23] | data[0x24] << 8;

    EventDataType epap = data[0x04] | (data[0x05] << 8);
    EventDataType ipap = data[0x06] | (data[0x07] << 8);
    // why is there another high setting?

    session->settings[CPAP_EPAP] = epap/10.0;
    session->settings[CPAP_IPAP] = ipap/10.0;
    session->settings[CPAP_Mode] = (CPAPMode)MODE_AVAPS;


//    EventDataType f1 = data[0x08] | (data[0x09] << 8);

    return true;
}

bool PRS1Import::ParseSummaryF5V0()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    CPAPMode cpapmode = MODE_UNKNOWN;

    int imin_epap = data[0x3];
    int imax_epap = data[0x4];
    int imin_ps = data[0x5];
    int imax_ps = data[0x6];
    int imax_pressure = data[0x2];

    cpapmode = MODE_ASV_VARIABLE_EPAP;

    session->settings[CPAP_Mode] = (int)cpapmode;

    if (cpapmode == MODE_CPAP) {
        session->settings[CPAP_Pressure] = imin_epap/10.0f;

    } else if (cpapmode == MODE_BILEVEL_FIXED) {
        session->settings[CPAP_EPAP] = imin_epap/10.0f;
        session->settings[CPAP_IPAP] = imax_epap/10.0f;

    } else if (cpapmode == MODE_ASV_VARIABLE_EPAP) {
        //int imax_ipap = imax_epap + imax_ps;
        int imin_ipap = imin_epap + imin_ps;

        session->settings[CPAP_EPAPLo] = imin_epap / 10.0f;
        session->settings[CPAP_EPAPHi] = imax_epap / 10.0f;
        session->settings[CPAP_IPAPLo] = imin_ipap / 10.0f;
        session->settings[CPAP_IPAPHi] = imax_pressure / 10.0f;
        session->settings[CPAP_PSMin] = imin_ps / 10.0f;
        session->settings[CPAP_PSMax] = imax_ps / 10.0f;
    }

    quint8 flex = data[0x0c];

    int flexlevel = flex & 0x03;
    FlexMode flexmode = FLEX_Unknown;

    flex &= 0xf8;
    bool split = false;

    if (flex & 0x40) {  // This bit defines the Flex setting for the CPAP component of the Split night
        split = true;
    }
    if (flex & 0x80) { // CFlex bit
        if (flex & 0x10) {
            flexmode = FLEX_RiseTime;
        } else if (flex & 8) { // Plus bit
            if (split || (cpapmode == MODE_CPAP)) {
                flexmode = FLEX_CFlexPlus;
            } else if (cpapmode == MODE_APAP) {
                flexmode = FLEX_AFlex;
            }
        } else {
            // CFlex bits refer to Rise Time on BiLevel machines
            flexmode = (cpapmode >= MODE_BILEVEL_FIXED) ? FLEX_BiFlex : FLEX_CFlex;
        }
    } else flexmode = FLEX_None;

    session->settings[PRS1_FlexMode] = (int)flexmode;
    session->settings[PRS1_FlexLevel] = (int)flexlevel;


    int ramp_time = data[0x0a];
    EventDataType ramp_pressure = float(data[0x0b]) / 10.0;

    session->settings[CPAP_RampTime] = (int)ramp_time;
    session->settings[CPAP_RampPressure] = ramp_pressure;

    session->settings[PRS1_HumidStatus] = (bool)(data[0x0d] & 0x80);        // Humidifier Connected
    session->settings[PRS1_HeatedTubing] = (bool)(data[0x0d] & 0x10);        // Heated Hose??
    session->settings[PRS1_HumidLevel] = (int)(data[0x0d] & 7);          // Humidifier Value

    summary_duration = data[0x18] | data[0x19] << 8;

    return true;
}

bool PRS1Import::ParseSummaryF5V1()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    CPAPMode cpapmode = MODE_UNKNOWN;

    int imin_epap = data[0x3];
    int imax_epap = data[0x4];
    int imin_ps = data[0x5];
    int imax_ps = data[0x6];
    int imax_pressure = data[0x2];

    cpapmode = MODE_ASV_VARIABLE_EPAP;

    session->settings[CPAP_Mode] = (int)cpapmode;

    if (cpapmode == MODE_CPAP) {
        session->settings[CPAP_Pressure] = imin_epap/10.0f;

    } else if (cpapmode == MODE_BILEVEL_FIXED) {
        session->settings[CPAP_EPAP] = imin_epap/10.0f;
        session->settings[CPAP_IPAP] = imax_epap/10.0f;

    } else if (cpapmode == MODE_ASV_VARIABLE_EPAP) {
        //int imax_ipap = imax_epap + imax_ps;
        int imin_ipap = imin_epap + imin_ps;

        session->settings[CPAP_EPAPLo] = imin_epap / 10.0f;
        session->settings[CPAP_EPAPHi] = imax_epap / 10.0f;
        session->settings[CPAP_IPAPLo] = imin_ipap / 10.0f;
        session->settings[CPAP_IPAPHi] = imax_pressure / 10.0f;
        session->settings[CPAP_PSMin] = imin_ps / 10.0f;
        session->settings[CPAP_PSMax] = imax_ps / 10.0f;
    }

    quint8 flex = data[0x0c];

    int flexlevel = flex & 0x03;
    FlexMode flexmode = FLEX_Unknown;

    flex &= 0xf8;
    bool split = false;

    if (flex & 0x40) {  // This bit defines the Flex setting for the CPAP component of the Split night
        split = true;
    }
    if (flex & 0x80) { // CFlex bit
        if (flex & 0x10) {
            flexmode = FLEX_RiseTime;
        } else if (flex & 8) { // Plus bit
            if (split || (cpapmode == MODE_CPAP)) {
                flexmode = FLEX_CFlexPlus;
            } else if (cpapmode == MODE_APAP) {
                flexmode = FLEX_AFlex;
            }
        } else {
            // CFlex bits refer to Rise Time on BiLevel machines
            flexmode = (cpapmode >= MODE_BILEVEL_FIXED) ? FLEX_BiFlex : FLEX_CFlex;
        }
    } else flexmode = FLEX_None;

    session->settings[PRS1_FlexMode] = (int)flexmode;
    session->settings[PRS1_FlexLevel] = (int)flexlevel;


    int ramp_time = data[0x0a];
    EventDataType ramp_pressure = float(data[0x0b]) / 10.0;

    session->settings[CPAP_RampTime] = (int)ramp_time;
    session->settings[CPAP_RampPressure] = ramp_pressure;

    session->settings[PRS1_HumidStatus] = (bool)(data[0x0d] & 0x80);        // Humidifier Connected
    session->settings[PRS1_HeatedTubing] = (bool)(data[0x0d] & 0x10);        // Heated Hose??
    session->settings[PRS1_HumidLevel] = (int)(data[0x0d] & 7);          // Humidifier Value

    summary_duration = data[0x18] | data[0x19] << 8;

    return true;
}

bool PRS1Import::ParseSummaryF5V2()
{
    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    //CPAPMode cpapmode = MODE_UNKNOWN;
    summary_duration = data[0x18] | data[0x19] << 8;

    return true;
}

bool PRS1Import::ParseSummaryF0V6()
{
    // DreamStation machines...

    // APAP models..

    const unsigned char * data = (unsigned char *)summary->m_data.constData();

    if (data[0x00] > 0) {
        return false;
    }

    session->set_first(qint64(summary->timestamp) * 1000L);

    CPAPMode cpapmode = MODE_UNKNOWN;

    int imin_epap = 0;
    //int imax_epap = 0;
    int imin_ps   = 0;
    int imax_ps   = 0;
    //int imax_pressure = 0;
    int min_pressure = 0;
    int max_pressure = 0;
    int duration  = 0;

    // in 'data', we start with 3 bytes that don't follow the pattern
    // pattern is varNumber, dataSize, dataValue(dataSize)
    // examples, 0x0d 0x02 0x28 0xC8  , or 0x0a 0x01 0x64,
    // first, verify that this dataSize is where we expect
    //     each var pair in headerblock should be (indexByte, valueByte)
    if ( 0x01 != summary->m_headerblock[1 * 2] ) {
		qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad datablock length";
        return false;  //nope, not here
    }
    int dataBlockSize = summary->m_headerblock[1 * 2 + 1];
    //int zero = 0;
    const unsigned char *dataPtr;

    //      start at 3rd byte ; did we go past the end? ; increment for dataSize + varNumberByte + dataSizeByte
    for ( dataPtr = data + 3; dataPtr < (data + 3 + dataBlockSize); dataPtr+= dataPtr[1] + 2) {
        switch( *dataPtr) {
        case 00: // mode?
            break;
        case 01: // ???
            break;
        case 10: // 0x0a
            cpapmode = MODE_CPAP;
            if (dataPtr[1] != 1) qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad CPAP value";
            imin_epap = dataPtr[2];
            break;
        case 13: // 0x0d
            cpapmode = MODE_APAP;
            if (dataPtr[1] != 2) qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad APAP value";
            min_pressure = dataPtr[2];
            max_pressure = dataPtr[3];
            break;
        case 14: // 0x0e  // <--- this is a total guess.. might be 3 and have a pressure support value
            cpapmode = MODE_BILEVEL_FIXED;
            if (dataPtr[1] != 2) qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad APAP value";
            min_pressure = dataPtr[2];
            max_pressure = dataPtr[3];
            imin_ps = max_pressure - min_pressure;
            break;
        case 15: // 0x0f
            cpapmode = MODE_BILEVEL_AUTO_VARIABLE_PS; //might be C_CHECK?
            if (dataPtr[1] != 4) qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad APAP value";
            min_pressure = dataPtr[2];
            max_pressure = dataPtr[3];
            imin_ps = dataPtr[4];
            imax_ps = dataPtr[5];
            break;
        case 0x10: // Auto Trial mode
            cpapmode = MODE_APAP;
            if (dataPtr[1] != 3) qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Bad APAP value";
            min_pressure = dataPtr[3];
            max_pressure = dataPtr[4];
            break;

        case 0x35:
            duration += ( dataPtr[3] << 8 ) + dataPtr[2];
            break;
//        case 3:
//            break;
        default:
            // have not found this before
            ;
         //   qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Unknown datablock value:" << (zero + *dataPtr) ;
        }
    }
    // now we encounter yet a different format of data
  /*  const unsigned char *data2Ptr = data + 3 + dataBlockSize;
    // pattern is byte/data, where length of data depends on value of 'byte'
    bool data2Done = false;
    while (!data2Done) {
        switch(*data2Ptr){
        case 0:
            //this appears to be the last one.  '0' plus 5 bytes **eats crc** without checking
            data2Ptr += 4;
            data2Ptr += 2; //this is the **CRC**??
            data2Done = true; //hope this is always there, since we don't have blocksize from header
            break;
        case 1:
            //don't know yet.  data size is the '1' plus 16 bytes
            data2Ptr += 5;
            break;
        case 2:
            //don't know yet.  data size is the '2' plus 16 bytes
            data2Ptr += 3;
            break;
        case 3:
            //don't know yet.  data size is the '3' plus 4 bytes
            // have seen multiple of these....may have to add them?
            data2Ptr += 5;
            break;
        case 4:
            // have seen multiple of these....may have to add them?
            duration = ( data2Ptr[3] << 8 ) + data2Ptr[2];
            data2Ptr += 3;
            break;
        case 5:
            //don't know yet.  data size is the '5' plus 4 bytes
            data2Ptr += 5;
            break;
        case 6:
            //don't know yet.  data size is the '5' plus 1 byte
            data2Ptr += 2;
            break;
        case 8:
            //don't know yet.  data size is the '8' plus 27 bytes (might be a '0' in here...not enough different types found yet)
            data2Ptr += 28;
            break;
        default:
            qDebug() << "PRS1Loader::ParseSummaryF0V6=" << "Unknown datablock2 value:" << (zero + *data2Ptr) ;
            break;
        }
    }*/
// need to populate summary->

    summary_duration = duration;
    session->settings[CPAP_Mode] = (int)cpapmode;
    if (cpapmode == MODE_CPAP) {
        session->settings[CPAP_Pressure] = imin_epap/10.0f;

    } else if (cpapmode == MODE_APAP) {
        session->settings[CPAP_PressureMin] = min_pressure/10.0f;
        session->settings[CPAP_PressureMax] = max_pressure/10.0f;
    } else if (cpapmode == MODE_BILEVEL_FIXED) {
        // Guessing here.. haven't seen BIPAP data.
        session->settings[CPAP_EPAP] = min_pressure/10.0f;
        session->settings[CPAP_IPAP] = max_pressure/10.0f;
        session->settings[CPAP_PS] = imin_ps/10.0f;
    } else if (cpapmode == MODE_BILEVEL_AUTO_VARIABLE_PS) {
        session->settings[CPAP_EPAPLo] = min_pressure/10.0f;
        session->settings[CPAP_IPAPHi] = max_pressure/10.0f;
        session->settings[CPAP_PSMin] = imin_ps/10.0f;
        session->settings[CPAP_PSMax] = imax_ps/10.0f;

    }

    return true;
}

bool PRS1Import::ParseSummary()
{

    // Family 0 = XPAP
    // Family 3 = BIPAP AVAPS
    // Family 5 = BIPAP AutoSV


    if (!summary) return false;

    session->setPhysMax(CPAP_LeakTotal, 120);
    session->setPhysMin(CPAP_LeakTotal, 0);
    session->setPhysMax(CPAP_Pressure, 25);
    session->setPhysMin(CPAP_Pressure, 4);
    session->setPhysMax(CPAP_IPAP, 25);
    session->setPhysMin(CPAP_IPAP, 4);
    session->setPhysMax(CPAP_EPAP, 25);
    session->setPhysMin(CPAP_EPAP, 4);
    session->setPhysMax(CPAP_PS, 25);
    session->setPhysMin(CPAP_PS, 0);

    switch (summary->family) {
    case 0:
        if (summary->familyVersion == 6) {
            return ParseSummaryF0V6();
        } else if (summary->familyVersion == 4) {
            return ParseSummaryF0V4();
        } else {
            return ParseSummaryF0();
        }
    case 3:
        return ParseSummaryF3();
        break;
    case 5:
        if (summary->familyVersion == 1) {
            return ParseSummaryF5V1();
        } else if (summary->familyVersion == 0) {
            return ParseSummaryF5V0();
        } else if (summary->familyVersion == 2) {
            return ParseSummaryF5V1();
        }
    default:
        ;
    }

    this->loader->saveMutex.lock();
    if (!mach->unsupported()) {
        this->loader->unsupported(mach);
    }
    this->loader->saveMutex.unlock();
    return false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // ASV Codes (Family 5) Recheck 17/10/2013
    // These are all confirmed off Encore reports

    //cpapmax=EventDataType(data[0x02])/10.0;   // Max Pressure in ASV machines
    //minepap=EventDataType(data[0x03])/10.0;   // Min EPAP
    //maxepap=EventDataType(data[0x04])/10.0;   // Max EPAP
    //minps=EventDataType(data[0x05])/10.0      // Min Pressure Support
    //maxps=EventDataType(data[0x06])/10.0      // Max Pressure Support

    //duration=data[0x1B] | data[0x1C] << 8)  // Session length in seconds

    //epap90=EventDataType(data[0x21])/10.0;    // EPAP 90%
    //epapavg=EventDataType(data[0x22])/10.0;   // EPAP Average
    //ps90=EventDataType(data[0x23])/10.0;      // Pressure Support 90%
    //psavg=EventDataType(data[0x24])/10.0;     // Pressure Support Average

    //TODO: minpb=data[0x] | data[0x] << 8;           // Minutes in PB
    //TODO: minleak=data[0x] | data[0x] << 8;         // Minutes in Large Leak
    //TODO: oa_cnt=data[0x] | data[0x] << 8;          // Obstructive events count

    //ca_cnt=data[0x2d] | data[0x2e] << 8;      // Clear Airway Events count
    //h_cnt=data[0x2f] | data[0x30] << 8;       // Hypopnea events count
    //fl_cnt=data[0x33] | data[0x34] << 8;      // Flow Limitation events count

    //avg_leak=EventDataType(data[0x35]);       // Average Leak
    //avgptb=EventDataType(data[0x36]);         // Average Patient Triggered Breaths %
    //avgbreathrate=EventDataType(data[0x37]);  // Average Breaths Per Minute
    //avgminvent=EventDataType(data[0x38]);     // Average Minute Ventilation
    //avg_tidalvol=EventDataType(data[0x39])*10.0;  // Average Tidal Volume
    //////////////////////////////////////////////////////////////////////////////////////////
}

bool PRS1Import::ParseEvents()
{
    bool res = false;
    if (!event) return false;
    switch (event->family) {
    case 0:
        res = ParseF0Events();
        break;
    case 3:
        res = ParseF3Events();
        break;
    case 5:
        res = ParseF5Events();
        break;
    default:
        qDebug() << "Unknown PRS1 familyVersion" << event->familyVersion;
        return false;
    }

    if (res) {
        if (session->count(CPAP_IPAP) > 0) {
//            if (session->settings[CPAP_Mode].toInt() != (int)MODE_ASV) {
//                session->settings[CPAP_Mode] = MODE_BILEVEL_FIXED;
//            }

//            if (session->settings[CPAP_PresReliefType].toInt() != PR_NONE) {
//                session->settings[CPAP_PresReliefType] = PR_BIFLEX;
//            }

//            EventDataType min = session->settings[CPAP_PressureMin].toDouble();
//            EventDataType max = session->settings[CPAP_PressureMax].toDouble();
//            session->settings[CPAP_EPAP] = min;
//            session->settings[CPAP_IPAP] = max;

//            session->settings[CPAP_PS] = max - min;
//            session->settings.erase(session->settings.find(CPAP_PressureMin));
//            session->settings.erase(session->settings.find(CPAP_PressureMax));

//            session->m_valuesummary.erase(session->m_valuesummary.find(CPAP_Pressure));
//            session->m_wavg.erase(session->m_wavg.find(CPAP_Pressure));
//            session->m_min.erase(session->m_min.find(CPAP_Pressure));
//            session->m_max.erase(session->m_max.find(CPAP_Pressure));
//            session->m_gain.erase(session->m_gain.find(CPAP_Pressure));

        } else {
            if (!session->settings.contains(CPAP_Pressure) && !session->settings.contains(CPAP_PressureMin)) {
                session->settings[CPAP_BrokenSummary] = true;

                //session->set_last(session->first());
                if (session->Min(CPAP_Pressure) == session->Max(CPAP_Pressure)) {
                    session->settings[CPAP_Mode] = MODE_CPAP; // no ramp
                    session->settings[CPAP_Pressure] = session->Min(CPAP_Pressure);
                } else {
                    session->settings[CPAP_Mode] = MODE_APAP;
                    session->settings[CPAP_PressureMin] = session->Min(CPAP_Pressure);
                    session->settings[CPAP_PressureMax] = 0; //session->Max(CPAP_Pressure);
                }

                //session->Set("FlexMode",PR_UNKNOWN);

            }
        }

    }
    return res;
}

bool PRS1Import::ParseOximetery()
{
    int size = oximetery.size();

    for (int i=0; i < size; ++i) {
        PRS1DataChunk * oxi = oximetery.at(i);
        int num = oxi->waveformInfo.size();

        int size = oxi->m_data.size();
        if (size == 0) {
            continue;
        }
        quint64 ti = quint64(oxi->timestamp) * 1000L;
        qint64 dur = qint64(oxi->duration) * 1000L;

        if (num > 1) {
            // Process interleaved samples
            QVector<QByteArray> data;
            data.resize(num);

            int pos = 0;
            do {
                for (int n=0; n < num; n++) {
                    int interleave = oxi->waveformInfo.at(n).interleave;
                    data[n].append(oxi->m_data.mid(pos, interleave));
                    pos += interleave;
                }
            } while (pos < size);

            if (data[0].size() > 0) {
                EventList * pulse = session->AddEventList(OXI_Pulse, EVL_Waveform, 1.0, 0.0, 0.0, 0.0, dur / data[0].size());
                pulse->AddWaveform(ti, (unsigned char *)data[0].data(), data[0].size(), dur);
            }

            if (data[1].size() > 0) {
                EventList * spo2 = session->AddEventList(OXI_SPO2, EVL_Waveform, 1.0, 0.0, 0.0, 0.0, dur / data[1].size());
                spo2->AddWaveform(ti, (unsigned char *)data[1].data(), data[1].size(), dur);
            }

        }
    }
    return true;
}

bool PRS1Import::ParseWaveforms()
{
    int size = waveforms.size();
    quint64 s1, s2;


    qint64 lastti=0;
    EventList * bnd = nullptr; // Breathing Not Detected

    for (int i=0; i < size; ++i) {
        PRS1DataChunk * waveform = waveforms.at(i);
        int num = waveform->waveformInfo.size();

        int size = waveform->m_data.size();
        if (size == 0) {
            continue;
        }
        quint64 ti = quint64(waveform->timestamp) * 1000L;
        quint64 dur = qint64(waveform->duration) * 1000L;

        quint64 diff = ti - lastti;
        if ((diff > 500) && (lastti != 0)) {
            if (!bnd) {
                bnd = session->AddEventList(PRS1_BND, EVL_Event);
            }
            bnd->AddEvent(ti, double(diff)/1000.0);
        }

        if (num > 1) {
            // Process interleaved samples
            QVector<QByteArray> data;
            data.resize(num);

            int pos = 0;
            do {
                for (int n=0; n < num; n++) {
                    int interleave = waveform->waveformInfo.at(n).interleave;
                    data[n].append(waveform->m_data.mid(pos, interleave));
                    pos += interleave;
                }
            } while (pos < size);

            s1 = data[0].size();
            s2 = data[1].size();

            if (s1 > 0) {
                EventList * flow = session->AddEventList(CPAP_FlowRate, EVL_Waveform, 1.0, 0.0, 0.0, 0.0, double(dur) / double(s1));
                flow->AddWaveform(ti, (char *)data[0].data(), data[0].size(), dur);
            }

            if (s2 > 0) {
                EventList * pres = session->AddEventList(CPAP_MaskPressureHi, EVL_Waveform, 0.1, 0.0, 0.0, 0.0, double(dur) / double(s2));
                pres->AddWaveform(ti, (unsigned char *)data[1].data(), data[1].size(), dur);
            }

        } else {
            // Non interleaved, so can process it much faster
            EventList * flow = session->AddEventList(CPAP_FlowRate, EVL_Waveform, 1.0, 0.0, 0.0, 0.0, double(dur) / double(waveform->m_data.size()));
            flow->AddWaveform(ti, (char *)waveform->m_data.data(), waveform->m_data.size(), dur);
        }
        lastti = dur+ti;
    }

    return true;
}

void PRS1Import::run()
{
    if (mach->unsupported())
        return;
    session = new Session(mach, sessionid);

    if ((compliance && ParseCompliance()) || (summary && ParseSummary())) {
        if (event && !ParseEvents()) {
        }
        waveforms = loader->ParseFile(wavefile);
        ParseWaveforms();

        oximetery = loader->ParseFile(oxifile);
        ParseOximetery();

        if (session->first() > 0) {
            if (session->last() < session->first()) {
                // if last isn't set, duration couldn't be gained from summary, parsing events or waveforms..
                // This session is dodgy, so kill it
                session->setSummaryOnly(true);
                session->really_set_last(session->first()+(qint64(summary_duration) * 1000L));
            }

            // Make sure it's saved
            session->SetChanged(true);

            // Add the session to the database
            loader->addSession(session);

            // Update indexes, process waveform and perform flagging
            session->UpdateSummaries();

            // Save is not threadsafe
            loader->saveMutex.lock();
            session->Store(mach->getDataPath());
            loader->saveMutex.unlock();

            // Unload them from memory
            session->TrashEvents();
        }

    }
}

QList<PRS1DataChunk *> PRS1Loader::ParseFile(QString path)
{
    QList<PRS1DataChunk *> CHUNKS;

    if (path.isEmpty())
        return CHUNKS;

    QFile f(path);

    if (!f.exists()) {
        return CHUNKS;
    }

    if (!f.open(QIODevice::ReadOnly)) {
        return CHUNKS;
    }

    PRS1DataChunk *chunk = nullptr, *lastchunk = nullptr;

    quint16 blocksize;
    quint16 wvfm_signals;

    unsigned char * header;
    int cnt = 0;

    //int lastheadersize = 0;
    int lastblocksize = 0;

    int cruft = 0;
    int firstsession = 0;


    do {
        QByteArray headerBA = f.read(16);
        if (headerBA.size() != 16) {
            break;
        }
        header = (unsigned char *)headerBA.data();

        blocksize = (header[2] << 8) | header[1];
        if (blocksize == 0) break;

        chunk = new PRS1DataChunk();

        chunk->sessionid = (header[10] << 24) | (header[9] << 16) | (header[8] << 8) | header[7];

        if (!firstsession) {
            firstsession = (int)chunk->sessionid;
        }
        chunk->fileVersion = header[0];
        chunk->htype = header[3];  // 00 = normal ?? // 01=waveform ?? // could be a bool signifying extra header bytes?
        chunk->family = header[4];
        chunk->familyVersion = header[5];
        chunk->ext = header[6];
        chunk->timestamp = (header[14] << 24) | (header[13] << 16) | (header[12] << 8) | header[11];

        if (lastchunk != nullptr) {
            if ((lastchunk->fileVersion != chunk->fileVersion)
                && (lastchunk->ext != chunk->ext)
                && (lastchunk->family != chunk->family)
                && (lastchunk->familyVersion != chunk->familyVersion)
                && (lastchunk->htype != chunk->htype)) {
                QByteArray junk = f.read(lastblocksize - 16);

                Q_UNUSED(junk)
                if (lastchunk->ext == 5) {
                    // The data is random crap
//                    lastchunk->m_data.append(junk.mid(lastheadersize-16));
                }
                delete chunk;
                ++cruft;
                if (cruft > 3)
                    break;

                continue;
                // Corrupt header.. skip it.
            }
        }

        int diff = 0;

        //////////////////////////////////////////////////////////
        // Family 3 (1060P)
        //////////////////////////////////////////////////////////
        if ((chunk->family == 3) && (chunk->ext == 2)) {
            QByteArray extra = f.read(47);
            if (extra.size() != 47) {
                delete chunk;
                break;
            }
            headerBA.append(extra);
            header = (unsigned char *)headerBA.data();
            chunk->m_headerblock = headerBA.right(48);

        }


        //////////////////////////////////////////////////////////
        // Waveform Header
        //////////////////////////////////////////////////////////
        if ((chunk->ext == 5) || (chunk->ext == 6)) {
            // Get extra 8 bytes in waveform header.
            QByteArray extra = f.read(4);
            if (extra.size() != 4) {
                delete chunk;
                break;
            }
            headerBA.append(extra);
            // Get the header address again to be safe
            header = (unsigned char *)headerBA.data();

            chunk->duration = header[0x0f] | header[0x10] << 8;

            wvfm_signals = header[0x12] | header[0x13] << 8;

            int ws_size = (chunk->fileVersion == 3) ? 4 : 3;

            int sbsize = wvfm_signals * ws_size + 1;
            QByteArray sbextra = f.read(sbsize);
            if (sbextra.size() != sbsize) {
                delete chunk;
                break;
            }
            headerBA.append(sbextra);
            header = (unsigned char *)headerBA.data();

            // Read the waveform information in reverse.
            int pos = 0x14 + (wvfm_signals - 1) * ws_size;
            for (int i = 0; i < wvfm_signals; ++i) {
                quint16 interleave = header[pos] | header[pos + 1] << 8; // samples per block (Usually 05 00)

                if (chunk->fileVersion == 2) {
                    quint8 sample_format = header[pos + 2];
                    chunk->waveformInfo.push_back(PRS1Waveform(interleave, sample_format));
                    pos -= 3;
                } else if (chunk->fileVersion == 3) {
                    //quint16 sample_size = header[pos + 2] | header[pos + 3] << 8; // size in bits?? (08 00)
                    // Possibly this is size in bits, and sign bit for the other byte?
                    chunk->waveformInfo.push_back(PRS1Waveform(interleave, 0));
                    pos -= 4;
                }
            }
            if (lastchunk != nullptr) {
                diff = (chunk->timestamp - lastchunk->timestamp) - lastchunk->duration;

            }
        }

        int headersize = headerBA.size();

        lastblocksize = blocksize;
        blocksize -= headersize;
        //lastheadersize = headersize;

        // Check header checksum

        quint8 csum = 0;
        for (int i=0; i < headersize-1; ++i) csum += header[i];

        if ((chunk->fileVersion==2) || (chunk->ext >= 5)) {
            if (csum != header[headersize-1]) {
                // header checksum error.
                delete chunk;
                return CHUNKS;
            }
        } else if ((chunk->fileVersion==3) && (chunk->ext <= 2)) {
            // DreamStation has an additional block of data following the timestamp, preceded by a length count,
            // followed by the additive checksum

            char len = header[headersize-1];

            csum += len;

            int h2len = len*2+1;

            blocksize -= h2len;

            // Read the extra data block
            chunk->m_headerblock = f.read(h2len);

            if (chunk->m_headerblock.size() < h2len) {
                delete chunk;
                return CHUNKS;
            }
            unsigned char * header2 = (unsigned char*) chunk->m_headerblock.data();

            // Checksum the whole header
            for (int i=0; i < h2len-1; ++i) csum += header2[i];

            if (csum != header2[h2len-1]) {
                // header checksum error.
                delete chunk;
                return CHUNKS;
            }

        } else {
            // uhhhh.. should not of got this far. because this is an unknown or corrupt file format.
            delete chunk;
            return CHUNKS;
        }


        // Read data block
        chunk->m_data = f.read(blocksize);

        if (chunk->m_data.size() < blocksize) {
            delete chunk;
            break;
        }

        if (chunk->fileVersion==3) {
            //int ds = chunk->m_data.size();
            //quint32 crc16 = chunk->m_data.at(ds-2) | chunk->m_data.at(ds-1) << 8;
            chunk->m_data.chop(4);
        } else {
            // last two bytes contain crc16 checksum.
            int ds = chunk->m_data.size();
            quint16 crc16 = chunk->m_data.at(ds-2) | chunk->m_data.at(ds-1) << 8;
            chunk->m_data.chop(2);
#ifdef PRS1_CRC_CHECK
            // This fails.. it needs to include the header!
            quint16 calc16 = CRC16((unsigned char *)chunk->m_data.data(), chunk->m_data.size());
            if (calc16 != crc16) {
                // corrupt data block.. bleh..
            //   qDebug() << "CRC16 doesn't match for chunk" << chunk->sessionid << "for" << path;
            }
#endif
        }


        if ((chunk->ext == 5) || (chunk->ext == 6)) {  // if Flow/MaskPressure Waveform or OXI Waveform file
            if (lastchunk != nullptr) {

                Q_ASSERT(lastchunk->sessionid == chunk->sessionid);

                if (diff == 0) {
                    // In sync, so append waveform data to previous chunk
                    lastchunk->m_data.append(chunk->m_data);
                    lastchunk->duration += chunk->duration;
                    delete chunk;
                    cnt++;
                    chunk = lastchunk;
                    continue;
                }
                // else start a new chunk to resync
            }
        }

        CHUNKS.append(chunk);

        lastchunk = chunk;
        cnt++;
    } while (!f.atEnd());
    return CHUNKS;
}

void InitModelMap()
{
    ModelMap[0x34] = QObject::tr("RemStar Pro with C-Flex+"); // 450/460P
    ModelMap[0x35] = QObject::tr("RemStar Auto with A-Flex"); // 550/560P
    ModelMap[0x36] = QObject::tr("RemStar BiPAP Pro with Bi-Flex");
    ModelMap[0x37] = QObject::tr("RemStar BiPAP Auto with Bi-Flex");
    ModelMap[0x38] = QObject::tr("RemStar Plus");          // 150/250P/260P
    ModelMap[0x41] = QObject::tr("BiPAP autoSV Advanced");
    ModelMap[0x4a] = QObject::tr("BiPAP autoSV Advanced 60 Series");
    ModelMap[0x4E] = QObject::tr("BiPAP AVAPS");
    ModelMap[0x58] = QObject::tr("CPAP");  // guessing
    ModelMap[0x59] = QObject::tr("CPAP Pro"); // guessing
    ModelMap[0x5A] = QObject::tr("Auto CPAP");
    ModelMap[0x5B] = QObject::tr("BiPAP Pro"); // guessing
    ModelMap[0x5C] = QObject::tr("Auto BiPAP");
}

bool initialized = false;

using namespace schema;

Channel PRS1Channels;

void PRS1Loader::initChannels()
{
    Channel * chan = nullptr;

    channel.add(GRP_CPAP, new Channel(CPAP_PressurePulse = 0x1009, MINOR_FLAG,  MT_CPAP,   SESSION,
        "PressurePulse",
        QObject::tr("Pressure Pulse"),
        QObject::tr("A pulse of pressure 'pinged' to detect a closed airway."),
        QObject::tr("PP"),
		UnitPerMinute::STR_UNIT_EventsPerHour,    DEFAULT,    QColor("dark red")));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_FlexMode = 0xe105, SETTING,  MT_CPAP,  SESSION,
        "PRS1FlexMode", QObject::tr("Flex Mode"),
        QObject::tr("PRS1 pressure relief mode."),
        QObject::tr("Flex Mode"),
        "", LOOKUP, Qt::green));


    chan->addOption(FLEX_None, STR_TR_None);
    chan->addOption(FLEX_CFlex, QObject::tr("C-Flex"));
    chan->addOption(FLEX_CFlexPlus, QObject::tr("C-Flex+"));
    chan->addOption(FLEX_AFlex, QObject::tr("A-Flex"));
    chan->addOption(FLEX_RiseTime, QObject::tr("Rise Time"));
    chan->addOption(FLEX_BiFlex, QObject::tr("Bi-Flex"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_FlexLevel = 0xe106, SETTING, MT_CPAP,   SESSION,
        "PRS1FlexSet",
        QObject::tr("Flex Level"),
        QObject::tr("PRS1 pressure relief setting."),
        QObject::tr("Flex Level"),
        "", LOOKUP, Qt::blue));

    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, QObject::tr("x1"));
    chan->addOption(2, QObject::tr("x2"));
    chan->addOption(3, QObject::tr("x3"));
    chan->addOption(4, QObject::tr("x4"));
    chan->addOption(5, QObject::tr("x5"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_HumidStatus = 0xe101, SETTING, MT_CPAP, SESSION,
        "PRS1HumidStat",
        QObject::tr("Humidifier Status"),
        QObject::tr("PRS1 humidifier connected?"),
        QObject::tr("Humidifier Status"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, QObject::tr("Disconnected"));
    chan->addOption(1, QObject::tr("Connected"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_HeatedTubing = 0xe10d, SETTING, MT_CPAP,  SESSION,
        "PRS1HeatedTubing",
        QObject::tr("Heated Tubing"),
        QObject::tr("Heated Tubing Connected"),
        QObject::tr("Heated Tubing"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, QObject::tr("Yes"));
    chan->addOption(1, QObject::tr("No"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_HumidLevel = 0xe102, SETTING,  MT_CPAP,  SESSION,
        "PRS1HumidLevel",
        QObject::tr("Humidification Level"),
        QObject::tr("PRS1 Humidification level"),
        QObject::tr("Humid. Lvl."),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, QObject::tr("x1"));
    chan->addOption(2, QObject::tr("x2"));
    chan->addOption(3, QObject::tr("x3"));
    chan->addOption(4, QObject::tr("x4"));
    chan->addOption(5, QObject::tr("x5"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_SysOneResistStat = 0xe103, SETTING, MT_CPAP,   SESSION,
        "SysOneResistStat",
        QObject::tr("System One Resistance Status"),
        QObject::tr("System One Resistance Status"),
        QObject::tr("Sys1 Resist. Status"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

    channel.add(GRP_CPAP, chan = new Channel(PRS1_SysOneResistSet = 0xe104, SETTING, MT_CPAP,   SESSION,
        "SysOneResistSet",
        QObject::tr("System One Resistance Setting"),
        QObject::tr("System One Mask Resistance Setting"),
        QObject::tr("Sys1 Resist. Set"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, QObject::tr("x1"));
    chan->addOption(2, QObject::tr("x2"));
    chan->addOption(3, QObject::tr("x3"));
    chan->addOption(4, QObject::tr("x4"));
    chan->addOption(5, QObject::tr("x5"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_HoseDiam = 0xe107, SETTING,  MT_CPAP,  SESSION,
        "PRS1HoseDiam",
        QObject::tr("Hose Diameter"),
        QObject::tr("Diameter of primary CPAP hose"),
        QObject::tr("Hose Diameter"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, QObject::tr("22mm"));
    chan->addOption(1, QObject::tr("15mm"));

    channel.add(GRP_CPAP, chan = new Channel(PRS1_SysOneResistStat = 0xe108, SETTING,  MT_CPAP,  SESSION,
        "SysOneLock",
        QObject::tr("System One Resistance Lock"),
        QObject::tr("Whether System One resistance settings are available to you."),
        QObject::tr("Sys1 Resist. Lock"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

    channel.add(GRP_CPAP, chan = new Channel(PRS1_AutoOn = 0xe109, SETTING, MT_CPAP,   SESSION,
        "PRS1AutoOn",
        QObject::tr("Auto On"),
        QObject::tr("A few breaths automatically starts machine"),
        QObject::tr("Auto On"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

    channel.add(GRP_CPAP, chan = new Channel(PRS1_AutoOff = 0xe10a, SETTING, MT_CPAP,   SESSION,
        "PRS1AutoOff",
        QObject::tr("Auto Off"),
        QObject::tr("Machine automatically switches off"),
        QObject::tr("Auto Off"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

    channel.add(GRP_CPAP, chan = new Channel(PRS1_MaskAlert = 0xe10b, SETTING,  MT_CPAP,  SESSION,
        "PRS1MaskAlert",
        QObject::tr("Mask Alert"),
        QObject::tr("Whether or not machine allows Mask checking."),
        QObject::tr("Mask Alert"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

    channel.add(GRP_CPAP, chan = new Channel(PRS1_MaskAlert = 0xe10c, SETTING, MT_CPAP,   SESSION,
        "PRS1ShowAHI",
        QObject::tr("Show AHI"),
        QObject::tr("Whether or not machine shows AHI via LCD panel."),
        QObject::tr("Show AHI"),
        "", LOOKUP, Qt::green));
    chan->addOption(0, STR_TR_Off);
    chan->addOption(1, STR_TR_On);

//    <channel id="0xe10e" class="setting" scope="!session" name="PRS1Mode" details="PAP Mode" label="PAP Mode" type="integer" link="0x1200">
//     <Option id="0" value="CPAP"/>
//     <Option id="1" value="Auto"/>
//     <Option id="2" value="BIPAP"/>
//     <Option id="3" value="AutoSV"/>
//    </channel>

    QString unknowndesc=QObject::tr("Unknown PRS1 Code %1");
    QString unknownname=QObject::tr("PRS1_%1");
    QString unknownshort=QObject::tr("PRS1_%1");

    channel.add(GRP_CPAP, new Channel(PRS1_00 = 0x1150, UNKNOWN, MT_CPAP,    SESSION,
        "PRS1_00",
        QString(unknownname).arg(0,2,16,QChar('0')),
        QString(unknowndesc).arg(0,2,16,QChar('0')),
        QString(unknownshort).arg(0,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));

    channel.add(GRP_CPAP, new Channel(PRS1_01 = 0x1151, UNKNOWN,  MT_CPAP,   SESSION,
        "PRS1_01",
        QString(unknownname).arg(1,2,16,QChar('0')),
        QString(unknowndesc).arg(1,2,16,QChar('0')),
        QString(unknownshort).arg(1,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));

    channel.add(GRP_CPAP, new Channel(PRS1_08 = 0x1152, UNKNOWN, MT_CPAP,    SESSION,
        "PRS1_08",
        QString(unknownname).arg(8,2,16,QChar('0')),
        QString(unknowndesc).arg(8,2,16,QChar('0')),
        QString(unknownshort).arg(8,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));

    channel.add(GRP_CPAP, new Channel(PRS1_0A = 0x1154, UNKNOWN, MT_CPAP,    SESSION,
        "PRS1_0A",
        QString(unknownname).arg(0xa,2,16,QChar('0')),
        QString(unknowndesc).arg(0xa,2,16,QChar('0')),
        QString(unknownshort).arg(0xa,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));
    channel.add(GRP_CPAP, new Channel(PRS1_0B = 0x1155, UNKNOWN,  MT_CPAP,   SESSION,
        "PRS1_0B",
        QString(unknownname).arg(0xb,2,16,QChar('0')),
        QString(unknowndesc).arg(0xb,2,16,QChar('0')),
        QString(unknownshort).arg(0xb,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));
    channel.add(GRP_CPAP, new Channel(PRS1_0C = 0x1156, UNKNOWN,  MT_CPAP,   SESSION,
        "PRS1_0C",
        QString(unknownname).arg(0xc,2,16,QChar('0')),
        QString(unknowndesc).arg(0xc,2,16,QChar('0')),
        QString(unknownshort).arg(0xc,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));
    channel.add(GRP_CPAP, new Channel(PRS1_0E = 0x1157, UNKNOWN, MT_CPAP,    SESSION,
        "PRS1_0E",
        QString(unknownname).arg(0xe,2,16,QChar('0')),
        QString(unknowndesc).arg(0xe,2,16,QChar('0')),
        QString(unknownshort).arg(0xe,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));
    channel.add(GRP_CPAP, new Channel(PRS1_BND = 0x1159, SPAN,  MT_CPAP,   SESSION,
        "PRS1_BND",
        QObject::tr("Breathing Not Detected"),
        QObject::tr("A period during a session where the machine could not detect flow."),
        QObject::tr("BND"),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("light purple")));

    channel.add(GRP_CPAP, new Channel(PRS1_15 = 0x115A, UNKNOWN,  MT_CPAP,   SESSION,
        "PRS1_15",
        QString(unknownname).arg(0x15,2,16,QChar('0')),
        QString(unknowndesc).arg(0x15,2,16,QChar('0')),
        QString(unknownshort).arg(0x15,2,16,QChar('0')),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));


    channel.add(GRP_CPAP, new Channel(PRS1_TimedBreath = 0x1180, MINOR_FLAG, MT_CPAP,    SESSION,
        "PRS1TimedBreath",
        QObject::tr("Timed Breath"),
        QObject::tr("Machine Initiated Breath"),
        QObject::tr("TB"),
        STR_UNIT_Unknown,
        DEFAULT,    QColor("black")));
}

void PRS1Loader::Register()
{
    if (initialized) { return; }

    qDebug() << "Registering PRS1Loader";
    RegisterLoader(new PRS1Loader());
    InitModelMap();
    initialized = true;
}

/* Thanks SleepyCPAP :)
CODE ERROR DESCRIPTION ERROR TYPE ERROR CATEGORY
1 SOFTWARE STOP STOP General Errors
2 Not Used General Errors
3 INT RAM REBOOT General Errors
4 NULL PTR REBOOT General Errors
5 DATA REBOOT General Errors
6 STATE MACHINE REBOOT General Errors
7 SOFTWARE REBOOT General Errors
8-9 Not Used General Errors
10 WDOG TEST RAM REBOOT Watchdog & Timer Errors
11 WDOG TEST REBOOT Watchdog & Timer Errors
12 BACKGROUND WDOG NO CARD REBOOT Watchdog & Timer Errors
13 BACKGROUND WDOG SD CARD REBOOT Watchdog & Timer Errors
14 WDOG LOWRES TIMER REBOOT Watchdog & Timer Errors
15 CYCLE HANDLER OVERRUN REBOOT Watchdog & Timer Errors
16 RASP RESTORE TIMEOUT CONTINUE Watchdog & Timer Errors
17 ONEMS HANDLER OVERRUN REBOOT Watchdog & Timer Errors
18 Not Used Watchdog & Timer Errors
19 WDOG TIMEOUT REBOOT Watchdog & Timer Errors
20 MOTOR SPINUP FLUX LOW REBOOT Motor/Blower Errors
21 MOTOR VBUS HIGH STOP Motor/Blower Errors
22 MOTOR FLUX MAGNITUDE REBOOT Motor/Blower Errors
23 MOTOR OVERSPEED REBOOT Motor/Blower Errors
24 MOTOR SPEED REVERSE REBOOT Motor/Blower Errors
25 MOTOR THERMISTOR OPEN CONTINUE Motor/Blower Errors
26 MOTOR THERMISTOR SHORTED CONTINUE Motor/Blower Errors
27 MOTOR RL NOCONVERGE STOP Motor/Blower Errors
28 NEGATIVE QUADRATURE VOLTAGE VECTOR REBOOT Motor/Blower Errors
29 VBUS GAIN ZERO: REBOOT Motor/Blower Errors
30 MOTOR SPINUP FLUX HIGH REBOOT Motor/Blower Errors
31 (incorrect power supply - 60series) Motor/Blower Errors
32-39 Not Used Motor/Blower Errors
40 NVRAM REBOOT NVRAM Low Level Errors
41 STORAGE UNIT RAM REBOOT NVRAM Low Level Errors
42 UNABLE TO OBTAIN BUS REBOOT NVRAM Low Level Errors
43 NVRAM NO CALLBACK OCCURRED REBOOT NVRAM Low Level Errors
44 NV BUFFER NULL REBOOT NVRAM Low Level Errors
45 NV CALLBACK NULL REBOOT NVRAM Low Level Errors
46 NV ZERO LENGTH REBOOT NVRAM Low Level Errors
47 NVRAM INVALID BYTES XFRRED REBOOT NVRAM Low Level Errors
48-49 Not Used NVRAM Low Level Errors
50 DAILY VALUES CORRUPT LOG ONLY NVRAM Unit Related Errors
51 CORRUPT COMPLIANCE LOG CONTINUE NVRAM Unit Related Errors
52 CORRUPT COMPLIANCE CB CONTINUE NVRAM Unit Related Errors
53 COMP LOG SEM TIMEOUT CONTINUE NVRAM Unit Related Errors
54 COMPLOG REQS OVERFLOW REBOOT NVRAM Unit Related Errors
55 THERAPY QUEUE FULL CONTINUE NVRAM Unit Related Errors
56 COMPLOG PACKET STATUS REBOOT NVRAM Unit Related Errors
57 SESS OBS QUEUE OVF REBOOT NVRAM Unit Related Errors
58 SESS OBS NO CALLBACK REBOOT NVRAM Unit Related Errors
59 Not Used NVRAM Unit Related Errors
60 UNSUPPORTED HARDWARE REBOOT General Hardware Errors
61 PLL UNLOCKED REBOOT General Hardware Errors
62 STUCK RAMP KEY CONTINUE General Hardware Errors
63 STUCK KNOB KEY CONTINUE General Hardware Errors
64 DSP OVERTIME PWM REBOOT General Hardware Errors
65 STUCK ENCODER A CONTINUE General Hardware Errors
66 STUCK ENCODER B CONTINUE General Hardware Errors
67-69 Not Used General Hardware Errors
70 PRESSURE SENSOR ABSENT STOP Pressure Sensor Errors
71 Not Used Pressure Sensor Errors
72 PSENS UNABLE TO OBTAIN BUS REBOOT Pressure Sensor Errors
73 SENSOR PRESS OFFSET STOP STOP Pressure Sensor Errors
74-79 Not Used Pressure Sensor Errors
80 UNABLE TO INIT FLOW SENSOR REBOOT Flow Sensor Errors
81 FLOW SENSOR TABLE CONTINUE Flow Sensor Errors
82 FLOW SENSOR OFFSET CONTINUE Flow Sensor Errors
83 FSENS UNABLE TO OBTAIN BUS REBOOT / 2nd failure=STOP Flow Sensor Errors
84 FLOW SENSOR STOP STOP Flow Sensor Errors
85 FLOW SENSOR OCCLUDED CONTINUE Flow Sensor Errors
86 FLOW SENSOR ABSENT CONTINUE Flow Sensor Errors
87 FLOW SENSOR BUS CONTINUE Flow Sensor Errors
88-89 Not Used Flow Sensor Errors
90 OTP NOT CONFIGURED STOP OTP & RTC Errors
91 OTP INCORRECTLY CONFIGURED STOP OTP & RTC Errors
92 Not Used OTP & RTC Errors
93 RTC VALUE CONTINUE OTP & RTC Errors
94 RTC STOPPED CONTINUE OTP & RTC Errors
95-99 Not Used OTP & RTC Errors
100 HUMID NO HEAT CONTINUE Humidifier Errors
101 HUMID TEMP MAX STOP Humidifier Errors
102 THERMISTOR HIGH CONTINUE Humidifier Errors
103 THERMISTOR LOW CONTINUE Humidifier Errors
104 HUMID AMBIENT OFF CONTINUE Humidifier Errors
105 HUMID AMBIENT COMM CONTINUE Humidifier Errors
106-109 Not Used Humidifier Errors
110 STACK REBOOT Stack & Exception Handler Errors
111 EXCEPTION STACK OVERFLOW REBOOT Stack & Exception Handler Errors
112 EXCEPTION STACK RESERVE LOG ONLY Stack & Exception Handler Errors
113 EXCEPTION STACK UNDERFLOW REBOOT Stack & Exception Handler Errors
114 FIQ STACK OVERFLOW REBOOT Stack & Exception Handler Errors
115 FIQ STACK RESERVE LOG ONLY Stack & Exception Handler Errors
116 FIQ STACK UNDERFLOW REBOOT Stack & Exception Handler Errors
117 IRQ STACK OVERFLOW REBOOT Stack & Exception Handler Errors
118 IRQ STACK RESERVE LOG ONLY Stack & Exception Handler Errors
119 IRQ STACK UNDERFLOW REBOOT Stack & Exception Handler Errors
120 SVC STACK OVERFLOW REBOOT Stack & Exception Handler Errors
121 SVC STACK RESERVE LOG ONLY Stack & Exception Handler Errors
122 SVC STACK UNDERFLOW REBOOT Stack & Exception Handler Errors
123 DATA ABORT EXCEPTION REBOOT Stack & Exception Handler Errors
124 PREFETCH EXCEPTION REBOOT Stack & Exception Handler Errors
125 ILLEGAL INSTRUCTION EXCEPTION REBOOT Stack & Exception Handler Errors
126 SWI ABORT EXCEPTION REBOOT Stack & Exception Handler Errors
*/

