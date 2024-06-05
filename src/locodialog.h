// locodialog.h - adapted for raidtcl project 2018 - 2023 by Rainer Müller

/***************************************************************************
                               LocoDialog.h
                             -----------------
    begin                : 11.11.2000
    copyright            : (C) 2000 by Markus Pfeiffer
    email                : mail@markus-pfeiffer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOCODIALOG_H
#define LOCODIALOG_H

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qfont.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qtextstream.h>

#include "config.h"

// where to find the icons, decoders and protocols
#define FUNC_PIX_DIR    RESOURCE_DIR "/funcpixmaps/"
#define LOCO_PIX_DIR    RESOURCE_DIR "/locopixmaps/"
#define DECODER_FILE 	RESOURCE_DIR "/decoders"
#define PROTOCOL_FILE   RESOURCE_DIR "/protocols"


enum buttonTypes
{
    PUSHBUTTON,
    SWITCH
};


struct Decoder {
    Decoder(QString _name, unsigned int fc, QString _protocols)
    { name = _name; functions = fc; protocols = _protocols; }
    QString name;
    unsigned int functions;
    QString protocols;
};

struct Protocol{
    Protocol(QString _name, unsigned int _steps, unsigned int ar)
    {name= _name; steps= _steps; addressrange= ar;}
    QString name;
    unsigned int steps;
    unsigned int addressrange;
};

struct FunktionTypes {
    int ftId;
    const char* ftName;
};

enum FunktionTypeCount {
    functiontypes = 100
};

/*maximum number of function buttons*/
enum {
    maxfbuttoncount = 32
};

static const FunktionTypes ftData[functiontypes] = {
    { 0, QT_TRANSLATE_NOOP("LocoDialog", "unused")},
    { 1, QT_TRANSLATE_NOOP("LocoDialog", "Stirnbeleuchtung")},
    { 2, QT_TRANSLATE_NOOP("LocoDialog", "Innenbeleuchtung")},
    { 3, QT_TRANSLATE_NOOP("LocoDialog", "Rücklicht")},
    { 4, QT_TRANSLATE_NOOP("LocoDialog", "Fernlicht")},
    { 5, QT_TRANSLATE_NOOP("LocoDialog", "Geräusch")},
    { 6, QT_TRANSLATE_NOOP("LocoDialog", "Pantograf")},
    { 7, QT_TRANSLATE_NOOP("LocoDialog", "Rauch")},
    { 8, QT_TRANSLATE_NOOP("LocoDialog", "Rangiergang")},
    { 9, QT_TRANSLATE_NOOP("LocoDialog", "Telexkupplung beidseitig")},
    {10, QT_TRANSLATE_NOOP("LocoDialog", "Horn")},
    {11, QT_TRANSLATE_NOOP("LocoDialog", "Schaffnerpfiff")},
    {12, QT_TRANSLATE_NOOP("LocoDialog", "Dampfpfeife")},
    {13, QT_TRANSLATE_NOOP("LocoDialog", "Glocke")},
    {14, QT_TRANSLATE_NOOP("LocoDialog", "Links/Rechts")},
    {15, QT_TRANSLATE_NOOP("LocoDialog", "Heben/Senken")},
    {16, QT_TRANSLATE_NOOP("LocoDialog", "Drehen links")},
    {17, QT_TRANSLATE_NOOP("LocoDialog", "Kranarm heben/senken")},
    {18, QT_TRANSLATE_NOOP("LocoDialog", "ABV")},
    {19, QT_TRANSLATE_NOOP("LocoDialog", "Pumpe")},
    {20, QT_TRANSLATE_NOOP("LocoDialog", "Bremsenquietschen")},
    {21, QT_TRANSLATE_NOOP("LocoDialog", "Schaltstufen")},
    {22, QT_TRANSLATE_NOOP("LocoDialog", "Generator")},
    {23, QT_TRANSLATE_NOOP("LocoDialog", "Betriebsgeräusch")},
    {24, QT_TRANSLATE_NOOP("LocoDialog", "Motor")},
    {25, QT_TRANSLATE_NOOP("LocoDialog", "Bahnhofsansage")},
    {26, QT_TRANSLATE_NOOP("LocoDialog", "Kohle schaufeln")},
    {27, QT_TRANSLATE_NOOP("LocoDialog", "Türen schließen")},
    {28, QT_TRANSLATE_NOOP("LocoDialog", "Türe öffnen")},
    {29, QT_TRANSLATE_NOOP("LocoDialog", "Lüftergeräusch")},
    {30, QT_TRANSLATE_NOOP("LocoDialog", "Lüfter")},
    {31, QT_TRANSLATE_NOOP("LocoDialog", "Feuerbüchse")},
    {32, QT_TRANSLATE_NOOP("LocoDialog", "Innenbeleuchtung")},
    {33, QT_TRANSLATE_NOOP("LocoDialog", "Tischlampe Ep. IV")},
    {34, QT_TRANSLATE_NOOP("LocoDialog", "Tischlampe Ep.III")},
    {35, QT_TRANSLATE_NOOP("LocoDialog", "Tischlampe Ep. II")},
    {36, QT_TRANSLATE_NOOP("LocoDialog", "Schüttelrost")},
    {37, QT_TRANSLATE_NOOP("LocoDialog", "Schienenstoß")},
    {38, QT_TRANSLATE_NOOP("LocoDialog", "Nummernschild")},
    {39, QT_TRANSLATE_NOOP("LocoDialog", "Betriebsgeräusch")},
    {40, QT_TRANSLATE_NOOP("LocoDialog", "Zuglaufschild")},
    {41, QT_TRANSLATE_NOOP("LocoDialog", "Führerstand hinten")},
    {42, QT_TRANSLATE_NOOP("LocoDialog", "Führerstand vorn")},
    {43, QT_TRANSLATE_NOOP("LocoDialog", "Kuppeln")},
    {44, QT_TRANSLATE_NOOP("LocoDialog", "Pufferstoß")},
    {45, QT_TRANSLATE_NOOP("LocoDialog", "Zugansage")},
    {46, QT_TRANSLATE_NOOP("LocoDialog", "Kranhaken")},
    {47, QT_TRANSLATE_NOOP("LocoDialog", "Blinklicht")},
    {48, QT_TRANSLATE_NOOP("LocoDialog", "Führerstandsbel.")},
    {49, QT_TRANSLATE_NOOP("LocoDialog", "Pressluft")},
    {50, QT_TRANSLATE_NOOP("LocoDialog", "F0")},
    {51, QT_TRANSLATE_NOOP("LocoDialog", "F1")},
    {52, QT_TRANSLATE_NOOP("LocoDialog", "F2")},
    {53, QT_TRANSLATE_NOOP("LocoDialog", "F3")},
    {54, QT_TRANSLATE_NOOP("LocoDialog", "F4")},
    {55, QT_TRANSLATE_NOOP("LocoDialog", "F5")},
    {56, QT_TRANSLATE_NOOP("LocoDialog", "F6")},
    {57, QT_TRANSLATE_NOOP("LocoDialog", "F7")},
    {58, QT_TRANSLATE_NOOP("LocoDialog", "F8")},
    {59, QT_TRANSLATE_NOOP("LocoDialog", "F9")},
    {60, QT_TRANSLATE_NOOP("LocoDialog", "F10")},
    {61, QT_TRANSLATE_NOOP("LocoDialog", "F11")},
    {62, QT_TRANSLATE_NOOP("LocoDialog", "F12")},
    {63, QT_TRANSLATE_NOOP("LocoDialog", "F13")},
    {64, QT_TRANSLATE_NOOP("LocoDialog", "F14")},
    {65, QT_TRANSLATE_NOOP("LocoDialog", "F15")},
    {66, QT_TRANSLATE_NOOP("LocoDialog", "F16")},
    {67, QT_TRANSLATE_NOOP("LocoDialog", "F17")},
    {68, QT_TRANSLATE_NOOP("LocoDialog", "F18")},
    {69, QT_TRANSLATE_NOOP("LocoDialog", "F19")},
    {70, QT_TRANSLATE_NOOP("LocoDialog", "F20")},
    {71, QT_TRANSLATE_NOOP("LocoDialog", "F21")},
    {72, QT_TRANSLATE_NOOP("LocoDialog", "F22")},
    {73, QT_TRANSLATE_NOOP("LocoDialog", "F23")},
    {74, QT_TRANSLATE_NOOP("LocoDialog", "F24")},
    {75, QT_TRANSLATE_NOOP("LocoDialog", "F25")},
    {76, QT_TRANSLATE_NOOP("LocoDialog", "F26")},
    {77, QT_TRANSLATE_NOOP("LocoDialog", "F27")},
    {78, QT_TRANSLATE_NOOP("LocoDialog", "F28")},
    {79, QT_TRANSLATE_NOOP("LocoDialog", "F29")},
    {80, QT_TRANSLATE_NOOP("LocoDialog", "F30")},
    {81, QT_TRANSLATE_NOOP("LocoDialog", "F31")},
    {82, QT_TRANSLATE_NOOP("LocoDialog", "Telexkupplung hinten")},
    {83, QT_TRANSLATE_NOOP("LocoDialog", "Telexkupplung vorne")},
    {84, QT_TRANSLATE_NOOP("LocoDialog", "Pantograf hinten")},
    {85, QT_TRANSLATE_NOOP("LocoDialog", "Pantograf vorne")},
    {86, QT_TRANSLATE_NOOP("LocoDialog", "Licht hinten")},
    {87, QT_TRANSLATE_NOOP("LocoDialog", "Licht vorne")},
    {88, QT_TRANSLATE_NOOP("LocoDialog", "Heben")},
    {89, QT_TRANSLATE_NOOP("LocoDialog", "Lüfter")},
    {90, QT_TRANSLATE_NOOP("LocoDialog", "Triebwerksbeleuchtung")},
    {91, QT_TRANSLATE_NOOP("LocoDialog", "Zylinder ausblasen")},
    {92, QT_TRANSLATE_NOOP("LocoDialog", "Dampfstoß")},
    {93, QT_TRANSLATE_NOOP("LocoDialog", "Kran")},
    {94, QT_TRANSLATE_NOOP("LocoDialog", "Auf")},
    {95, QT_TRANSLATE_NOOP("LocoDialog", "Ab")},
    {96, QT_TRANSLATE_NOOP("LocoDialog", "Links")},
    {97, QT_TRANSLATE_NOOP("LocoDialog", "Rechts")},
    {98, QT_TRANSLATE_NOOP("LocoDialog", "Drehen rechts")},
    {99, QT_TRANSLATE_NOOP("LocoDialog", "Magnet")}
/*
    {0, QT_TRANSLATE_NOOP("LocoDialog", "Light"),    "light"},
    {1, QT_TRANSLATE_NOOP("LocoDialog", "Smoke"),    "smoke"},
    {2, QT_TRANSLATE_NOOP("LocoDialog", "Coupling"), "coupl"},
    {3, QT_TRANSLATE_NOOP("LocoDialog", "Sound"),    "sound"},
    {4, QT_TRANSLATE_NOOP("LocoDialog", "Shunting"), "shunt"},
    {5, QT_TRANSLATE_NOOP("LocoDialog", "Cabinlight"), "cabinlight"},
    {6, QT_TRANSLATE_NOOP("LocoDialog", "Wagonlight"), "wagonlight"},
    {7, QT_TRANSLATE_NOOP("LocoDialog", "Gearlight"), "gearlight"},
    {8, QT_TRANSLATE_NOOP("LocoDialog", "Whistle"), "whistle"}
*/
};
// TODO: add
//("bright light");
//("breaking");
//("turbines"); Lüfter
//Pantograph

struct Function {
    Function(int ft, int bt, bool a) {
        active = a;
        buttontype = bt;
        functiontype = ft;
    }
	bool active; int buttontype; int functiontype;
};

// structure to collect function settings widgets
struct FunctionWidgets {
    FunctionWidgets(QHBoxLayout* l, QLabel* fl, 	// QCheckBox* v,
            QLabel* tl, QComboBox* i, QLabel*al, QComboBox* b) {
        lineLayout = l;
        fLabel = fl;
        typeLabel = tl;
        functiontypeCB = i;
        activateLabel = al;
        buttontypeCB = b;
    }
    QHBoxLayout* lineLayout;
    QLabel* fLabel;
    QLabel* typeLabel;
    QComboBox* functiontypeCB;
    QLabel* activateLabel;
    QComboBox* buttontypeCB;
};


class LocoDialog: public QDialog
{
    Q_OBJECT

    int          maxAddress;

    QList<Decoder> decoders;
    QList<Protocol> protocols;
    QList<FunctionWidgets> functionwidgets;

    QLineEdit    *leName;
    QLineEdit    *leAlias;
    QSpinBox     *spAddress;
    QLineEdit    *locoIcon;
    QLineEdit    *leRealMaxSpeed;
    QSlider      *sliderMaxSpeed;
    QSlider      *sliderAvgSpeed;
    QSlider      *sliderMinSpeed;
    QSlider      *sliderAccelTime;
    QSlider      *sliderBreakTime;
    QVBoxLayout  *functionLayout;
    QComboBox    *decoderCB;
    QComboBox    *protocolCB;
    QLineEdit	 *leMfxuid;
    QPushButton  *bindPB;
    QCheckBox	 *cbReverseDirection;
    QRadioButton *rbUnitkm;
    QTabWidget   *tabs;
    QLabel	 *labelDescription;
    QCheckBox	 *keepLimitCB;
    QCheckBox	 *followLimitCB;

    void setupTabGeneral();
    void setupTabFuncs();
    void setupTabSpeeds();
    void setupTabCrcf();
    void updateProtocolValues(const Protocol&);
    bool loadDecoderFile();
    bool loadProtocolFile();
    void printDescription(const Protocol&);
    void updateFunctionView(int);
    void addFunctionLine(int);
    void removeFunctionLine();

public:
    LocoDialog(QWidget* parent = 0);

    int exec();
    void setLocoName(const QString&);
    QString getLocoName() const;
    void setTrainNumber(unsigned int);
    unsigned int getTrainNumber();
    void setLocoPixmapName(const QString&);
    QString getLocoPixmapName() const;
    void setReverseDirection(bool);
    bool getReverseDirection();
    void setDecoder(const QString&, const QString&);
    void getDecoder(QString&, QString&);
    void setAddress(int);
    int getAddress();
    void setMfxUid(unsigned int);
    void setAttenuationTimes(int, int);
    void getAttenuationTimes(int&, int&);
    void setLocoSpeeds(int, int, int, int);
    void getLocoSpeeds(int&, int&, int&, int&);
    void setSpeedUnitKmh(bool);
    bool getSpeedUnitKmh();
    void setFunctions(const QList<Function>&);
    void getFunctions(QList<Function>&);
    void setKeepLimit(bool);
    bool getKeepLimit();
    void setFollowLimit(bool);
    bool getFollowLimit();

private slots:
    void verify();
    void slotGetIcon();
    void updateProtocolData(int);
    void updateProtocolDescription(int);
    void updateProtocolCB(int);
};

#endif    //LOCODIALOG_H
