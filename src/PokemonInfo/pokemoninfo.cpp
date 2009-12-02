#include "pokemoninfo.h"
#include "pokemonstructs.h"
#include "../../SpecialIncludes/zip.h"

/*initialising static variables */
QString PokemonInfo::m_Directory;
QStringList PokemonInfo::m_Names;

QString MoveInfo::m_Directory;
QStringList MoveInfo::m_Names;
QStringList MoveInfo::m_AccS;
QStringList MoveInfo::m_PowerS;
QStringList MoveInfo::m_SpecialEffects;
QList<char> MoveInfo::m_Type;
QList<char> MoveInfo::m_PP;
QList<char> MoveInfo::m_Category;
QStringList MoveInfo::m_Effects;
QList<char> MoveInfo::m_Critical;
QList<char> MoveInfo::m_EffectRate;
QList<bool> MoveInfo::m_Physical;
QList<bool> MoveInfo::m_KingRock;
QList<char> MoveInfo::m_Speeds;
QList<int> MoveInfo::m_Flinch;
QList<int> MoveInfo::m_Recoil;
QList<QPair<char, char> > MoveInfo::m_Repeat;

QString ItemInfo::m_Directory;
QStringList ItemInfo::m_Names;
QStringList ItemInfo::m_SortedNames;

QStringList TypeInfo::m_Names;
QList<QColor> TypeInfo::m_Colors;
QString TypeInfo::m_Directory;
QList<int> TypeInfo::m_TypeVsType;

QStringList NatureInfo::m_Names;
QString NatureInfo::m_Directory;

QStringList CategoryInfo::m_Names;
QList<QColor> CategoryInfo::m_Colors;
QString CategoryInfo::m_Directory;

QStringList AbilityInfo::m_Names;
QString AbilityInfo::m_Directory;

QStringList GenderInfo::m_Names;
QList<QPixmap> GenderInfo::m_Pictures;
QString GenderInfo::m_Directory;

QString HiddenPowerInfo::m_Directory;

QString StatInfo::m_Directory;
QStringList StatInfo::m_stats;
QStringList StatInfo::m_status;

QByteArray readZipFile(const char *archiveName, const char *fileName)
{
    int error = 0;
    zip *archive;
    zip_file *file;
    char buffer[1024];
    int readsize = 0;
    QByteArray ret;

    archive = zip_open(archiveName, 0, &error);

    if (!archive)
    {
        qDebug() << "Error when opening archive" << archiveName;
        return ret;
    }

    file = zip_fopen(archive, fileName, 0);

    if (!file)
    {
        qDebug() << "Error when opening file "<< fileName <<" in archive: " << archiveName <<" : " << zip_strerror(archive);
        zip_close(archive);
        return ret;
    }

    do
    {
        ret.append(buffer, readsize);

        readsize = zip_fread(file, buffer, 1024);
    } while (readsize > 0) ;

    if (readsize < 0)
    {
        qDebug() << "Error when reading file "<< fileName <<" in archive: " << archiveName <<" : " << zip_file_strerror(file);
    }

    zip_fclose(file);
    zip_close(archive);

    return ret;
}

QString get_line(const QString & filename, int linenum)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    for (int i = 0; i < linenum; i++)
    {
        filestream.readLine();
    }

    return filestream.readLine();
}

template <class T>
void fill_container_with_file(T &container, const QString & filename)
{
    QFile file(filename);

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream filestream(&file);

    /* discarding all the uninteresting lines, should find a more effective way */
    while (!filestream.atEnd())
    {
        container << filestream.readLine();
    }
}

int PokemonInfo::Type1(int pokenum)
{
    return get_line(path("poke_type1.txt"), pokenum).toInt();
}

int PokemonInfo::Type2(int pokenum)
{
    return get_line(path("poke_type2.txt"), pokenum).toInt();
}

int PokemonInfo::calc_stat(quint8 basestat, int level, quint8 dv, quint8 ev)
{
    return ((2*basestat + dv+ ev/4)*level)/100 + 5;
}

int PokemonInfo::Stat(int stat, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp)
	return calc_stat(basestat, level, dv, ev) + level + 5;
    else
	return calc_stat(basestat, level, dv, ev);
}

int PokemonInfo::FullStat(int nature, int stat, quint8 basestat, int level, quint8 dv, quint8 ev)
{
    if (stat == Hp) {
	return Stat(stat, basestat, level, dv, ev);
    }
    else {
	return Stat(stat, basestat, level, dv, ev) * (10+NatureInfo::Boost(nature, stat)) / 10;
    }
}

void PokemonInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfPokemons() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

int PokemonInfo::NumberOfPokemons()
{
    return m_Names.size();
}

QString PokemonInfo::Name(int pokenum)
{
    return m_Names[pokenum];
}

int PokemonInfo::Number(const QString &pokename)
{
    return (qFind(m_Names.begin(), m_Names.end(), pokename)-m_Names.begin()) % (NumberOfPokemons());
}

int PokemonInfo::Gender(int pokenum)
{
    return Pokemon::GenderAvail(get_line(path("poke_gender.txt"), pokenum).toInt());
}

QPixmap PokemonInfo::Picture(int pokenum, int gender, bool shiney, bool back)
{
    QString archive = path("poke_img.zip");

    QString file = QString("%2/DP%3%4%5.png").arg(pokenum).arg(back?"b":"",(gender==Pokemon::Female)?"f":"m", shiney?"s":"");

    QByteArray data = readZipFile(archive.toLocal8Bit(), file.toLocal8Bit());

    if (data.length()==0)
	return QPixmap();

    QPixmap ret;
    ret.loadFromData(data, "png");

    return ret;
}

QIcon PokemonInfo::Icon(int index)
{
    QString archive = path("icons.zip");
    QString file = QString("gifs/%1.gif").arg(index);

    QByteArray data = readZipFile(archive.toLocal8Bit(),file.toLocal8Bit());
    if(data.length() == 0)
    {
        qDebug() << "erreur chargement fichier icon";
        return QIcon();
    }
    QPixmap p;
    p.loadFromData(data,"gif");
    QIcon ico(p);
    return ico;
}

QList<int> PokemonInfo::Moves(int pokenum)
{
    return LevelMoves(pokenum) << EggMoves(pokenum) << TutorMoves(pokenum) << TMMoves(pokenum) << SpecialMoves(pokenum);
}

QList<int> PokemonInfo::EggMoves(int pokenum)
{
    return getMoves("pokes_DP_eggmoves.txt", pokenum);
}

QList<int> PokemonInfo::LevelMoves(int pokenum)
{
    return getMoves("pokes_DP_lvmoves.txt", pokenum);
}

QList<int> PokemonInfo::TutorMoves(int pokenum)
{
    return getMoves("pokes_DP_MTmoves.txt", pokenum);
}

QList<int> PokemonInfo::TMMoves(int pokenum)
{
    return getMoves("pokes_DP_TMmoves.txt", pokenum);
}

QList<int> PokemonInfo::SpecialMoves(int pokenum)
{
    return getMoves("pokes_DP_specialmoves.txt", pokenum);
}

QList<int> PokemonInfo::Abilities(int pokenum)
{
    QList<int> ret;
    ret << get_line(path("poke_ability.txt"), pokenum).toInt() << get_line(path("poke_ability2.txt"), pokenum).toInt();

    return ret;
}

PokeBaseStats PokemonInfo::BaseStats(int pokenum)
{
    QString stats = get_line(path("poke_stats.txt"), pokenum);
    QTextStream statsstream(&stats, QIODevice::ReadOnly);

    int hp, att, def, spd, satt, sdef;

    statsstream >> hp >> att >> def >> spd >> satt >> sdef;

    return PokeBaseStats(hp, att, def, spd, satt, sdef);
}

void PokemonInfo::loadNames()
{
    fill_container_with_file(m_Names, path("pokemons_en.txt"));
}

QString PokemonInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

QList<int> PokemonInfo::getMoves(const QString &filename, int pokenum)
{
    QList<int> return_value;

    /* getting the line we want */
    QString interesting_line = get_line(path(filename), pokenum);

    /* extracting the moves */
    for (int i = 0; i + 3 <= interesting_line.length(); i+=3)
    {
        return_value << interesting_line.mid(i,3).toUInt();
    }

    return return_value;
}

void MoveInfo::loadCritics()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_critical.txt"));

    foreach(QString str, temp) {
	m_Critical.push_back(str.toInt());
    }
}

void MoveInfo::loadEffectRates()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_effect_rate.txt"));

    foreach(QString str, temp) {
	m_EffectRate.push_back(str.toInt());
    }
}

void MoveInfo::loadPhysics()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_physical_contact.txt"));

    foreach(QString str, temp) {
	m_Physical.push_back(str.toInt());
    }
}

void MoveInfo::loadKingRocks()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_kingrock.txt"));

    foreach(QString str, temp) {
	m_KingRock.push_back(str.toInt());
    }
}

void MoveInfo::loadRepeats()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_repeat.txt"));

    foreach(QString str, temp) {
	m_Repeat.push_back(QPair<char, char>(str[0].toAscii()-'0', str[2].toAscii()-'0'));
    }
}

void MoveInfo::loadSpeeds()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_speed_priority.txt"));

    foreach(QString str, temp) {
	m_Speeds.push_back(str.toInt());
    }
}

void MoveInfo::loadRecoil()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_recoil.txt"));

    foreach(QString str, temp) {
        m_Recoil.push_back(str.toInt());
    }
}

int MoveInfo::Recoil(int num)
{
    return m_Recoil[num];
}

void MoveInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfMoves() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadTypes();
    loadPPs();
    loadAccs();
    loadPowers();
    loadCategorys();
    loadEffects();
    loadRepeats();
    loadCritics();
    loadPhysics();
    loadKingRocks();
    loadEffectRates();
    loadSpeeds();
    loadFlinchs();
    loadRecoil();
    loadSpecialEffects();
}

int MoveInfo::NumberOfMoves()
{
    return m_Names.size();
}

int MoveInfo::Number(const QString &movename)
{
    return (qFind(m_Names.begin(), m_Names.end(), movename)-m_Names.begin()) % (NumberOfMoves());
}

QString MoveInfo::SpecialEffect(int movenum)
{
    return m_SpecialEffects[movenum];
}

void MoveInfo::loadNames()
{
    fill_container_with_file(m_Names, path("moves_en.txt"));
}

void MoveInfo::loadPPs()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_pp.txt"));
    foreach(QString s, temp) {
	m_PP.push_back(s.toInt());
    }
}

void MoveInfo::loadTypes()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_type.txt"));
    foreach(QString s, temp) {
	m_Type.push_back(s.toInt());
    }
}

void MoveInfo::loadCategorys()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_category.txt"));
    foreach(QString s, temp) {
	m_Category.push_back(s.toInt());
    }
}

void MoveInfo::loadPowers()
{
    fill_container_with_file(m_PowerS, path("move_power.txt"));
}

void MoveInfo::loadAccs()
{
    fill_container_with_file(m_AccS, path("move_accuracy.txt"));
}

QString MoveInfo::path(const QString &file)
{
    return m_Directory+file;
}

QString MoveInfo::Name(int movenum)
{
    return m_Names[movenum];
}

int MoveInfo::Power(int movenum)
{
    return PowerS(movenum).toInt();
}

int MoveInfo::Type(int movenum)
{
    return m_Type[movenum];
}

int MoveInfo::Category(int movenum)
{
    return m_Category[movenum];
}

int MoveInfo::PP(int movenum)
{
    return m_PP[movenum];
}

int MoveInfo::Acc(int movenum)
{
    int ret = AccS(movenum).toInt();
    return ret == 0 ? 65535 : ret;
}

QString MoveInfo::AccS(int movenum)
{
    return m_AccS[movenum];
}

QString MoveInfo::PowerS(int movenum)
{
    return m_PowerS[movenum];
}

QString MoveInfo::Effect(int movenum)
{
    return m_Effects[movenum];
}

int MoveInfo::CriticalRaise(int num)
{
    return m_Critical[num];
}

int MoveInfo::EffectRate(int num)
{
    return m_EffectRate[num];
}

bool MoveInfo::PhysicalContact(int num)
{
    return m_Physical[num];
}

bool MoveInfo::KingRock(int num)
{
    return m_KingRock[num];
}

int MoveInfo::RepeatMin(int num)
{
    return m_Repeat[num].first;
}

int MoveInfo::RepeatMax(int num)
{
    return m_Repeat[num].second;
}

int MoveInfo::SpeedPriority(int num)
{
    return m_Speeds[num];
}

int MoveInfo::FlinchRate(int num)
{
    return m_Flinch[num];
}

void MoveInfo::loadFlinchs()
{
    QList<QString> boogie;
    fill_container_with_file(boogie, path("move_flinch.txt"));

    foreach(QString str, boogie) {
        m_Flinch.push_back(str.toInt());
    }
}

void MoveInfo::loadEffects()
{
    QStringList temp;
    fill_container_with_file(temp, path("moveeffects.txt"));

    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
	m_Effects.push_back(eff.split('#').front());
    }
}

void MoveInfo::loadSpecialEffects()
{
    QStringList temp;
    fill_container_with_file(temp, path("move_special_effects.txt"));

    /* Removing comments, aka anything starting from '#' */
    foreach (QString eff, temp) {
	m_SpecialEffects.push_back(eff.split('#').front());
    }
}

void ItemInfo::init(const QString &dir)
{
    /* makes sure it isn't already initialized */
    if (NumberOfItems() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

void ItemInfo::loadNames()
{
    fill_container_with_file(m_Names, path("items_en.txt"));
    fill_container_with_file(m_Names, path("berries_en.txt"));

    m_SortedNames = m_Names;
    qSort(m_SortedNames);
}

QString ItemInfo::path(const QString &file)
{
    return m_Directory + file;
}

int ItemInfo::NumberOfItems()
{
    return m_Names.size();
}

QString ItemInfo::Name(int itemnum)
{
    return m_Names[itemnum];
}

int ItemInfo::Number(const QString &itemname)
{
    return (qFind(m_Names.begin(), m_Names.end(), itemname)-m_Names.begin()) % (NumberOfItems());
}

int ItemInfo::SortedNumber(const QString &itemname)
{
    return (qFind(m_SortedNames.begin(), m_SortedNames.end(), itemname)-m_SortedNames.begin()) % (NumberOfItems());
}

QStringList ItemInfo::Names()
{
    return m_Names;
}

QStringList ItemInfo::SortedNames()
{
    return m_SortedNames;
}

void TypeInfo::loadNames()
{
    fill_container_with_file(m_Names, path("types_en.txt"));
}

QString TypeInfo::path(const QString& file)
{
    return m_Directory+file;
}

void TypeInfo::loadColors()
{
    fill_container_with_file(m_Colors, path("type_colors.txt"));
}

void TypeInfo::loadEff()
{
    QStringList temp;

    fill_container_with_file(temp, path("typestable.txt"));

    foreach (QString l, temp) {
	QStringList l2 = l.split(' ');
	foreach (QString l3, l2) {
	    m_TypeVsType.push_back(l3.toInt());
	}
    }
}

void TypeInfo::init(const QString &dir)
{
    if (NumberOfTypes() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadColors();
    loadEff();
}

int TypeInfo::Eff(int type_attack, int type_defend)
{
    return m_TypeVsType[type_attack * NumberOfTypes() + type_defend];
}

QString TypeInfo::Name(int typenum)
{
    return m_Names[typenum];
}

QColor TypeInfo::Color(int typenum)
{
    return m_Colors[typenum];
}

int TypeInfo::NumberOfTypes()
{
    return m_Names.size();
}

void NatureInfo::loadNames()
{
    fill_container_with_file(m_Names, path("nature_en.txt"));
}

QString NatureInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

void NatureInfo::init(const QString &dir)
{
    if (NumberOfNatures() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

QString NatureInfo::Name(int naturenum)
{
    return m_Names[naturenum];
}

int NatureInfo::NumberOfNatures()
{
    return m_Names.size();
}

int NatureInfo::Boost(int nature, int stat)
{
    return -(nature%5 == stat-1) + (nature/5 == stat-1);
}


void CategoryInfo::loadNames()
{
    fill_container_with_file(m_Names, path("categories_en.txt"));
}

QString CategoryInfo::path(const QString& file)
{
    return m_Directory+file;
}

void CategoryInfo::loadColors()
{
    fill_container_with_file(m_Colors, path("category_colors.txt"));
}

void CategoryInfo::init(const QString &dir)
{
    if (NumberOfCategories() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadColors();
}

QString CategoryInfo::Name(int catnum)
{
    return m_Names[catnum];
}

QColor CategoryInfo::Color(int catnum)
{
    return m_Colors[catnum];
}

int CategoryInfo::NumberOfCategories()
{
    return m_Names.size();
}

void AbilityInfo::loadNames()
{
    fill_container_with_file(m_Names, path("abilities_en.txt"));
}

QString AbilityInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

void AbilityInfo::init(const QString &dir)
{
    if (NumberOfAbilities() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
}

QString AbilityInfo::Name(int abnum)
{
    return m_Names[abnum];
}

int AbilityInfo::NumberOfAbilities()
{
    return m_Names.size();
}


void GenderInfo::loadNames()
{
    fill_container_with_file(m_Names, path("genders_en.txt"));
}

void GenderInfo::loadPixmaps()
{
    for (int i = 0; i < NumberOfGenders(); i++) {
        m_Pictures << QPixmap(path(QString("gender%1.png").arg(i)));
    }
}

QString GenderInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

void GenderInfo::init(const QString &dir)
{
    if (NumberOfGenders() != 0)
        return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    loadNames();
    loadPixmaps();
}

QString GenderInfo::Name(int abnum)
{
    return m_Names[abnum];
}

QPixmap GenderInfo::Picture(int gender)
{
    return m_Pictures[gender];
}

int GenderInfo::NumberOfGenders()
{
    return m_Names.size();
}

void HiddenPowerInfo::init(const QString &dir)
{
    m_Directory = dir;
}

QString HiddenPowerInfo::path(const QString &filename)
{
    return m_Directory + filename;
}

int HiddenPowerInfo::Type(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 speed_dv, quint8 satt_dv, quint8 sdef_dv)
{
    return (((hp_dv%2) + (att_dv%2)*2 + (def_dv%2)*4 + (speed_dv%2)*8 + (satt_dv%2)*16 + (sdef_dv%2)*32)*15)/63 + 1;
}

int HiddenPowerInfo::Power(quint8 hp_dv, quint8 att_dv, quint8 def_dv, quint8 speed_dv, quint8 satt_dv, quint8 sdef_dv)
{
    return (((hp_dv%4>1) + (att_dv%4>1)*2 + (def_dv%4>1)*4 + (speed_dv%4>1)*8 + (satt_dv%4>1)*16 + (sdef_dv%4>1)*32)*40)/63 + 30;
}

QList<QStringList> HiddenPowerInfo::PossibilitiesForType(int type)
{
    QList<QString> fileLines;

    fill_container_with_file(fileLines, path(QString("type%1_hp.txt").arg(type)));

    QList<QStringList> ret;

    foreach (QString line, fileLines)
        ret.push_back(line.split(' '));

    return ret;
}

void StatInfo::init(const QString &dir)
{
    if (m_stats.size() != 0)
	return;

    m_Directory = dir;

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    fill_container_with_file(m_stats, path("stats_en.txt"));
    fill_container_with_file(m_status, path("status_en.txt"));
}

QString StatInfo::Stat(int stat)
{
    return m_stats[stat];
}

QString StatInfo::Status(int stat)
{
    return m_status[stat+1];
}

QString StatInfo::path(const QString &filename)
{
    return m_Directory + filename;
}
