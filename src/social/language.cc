/*************************************************************************
 *  File: language.cc                                                    *
 *  Usage: Source file for multi-language support                        *
 *                                                                       *
 *  All rights reserved.  See license.doc for complete information.      *
 *                                                                       *
 *********************************************************************** */

//
// File: language.cc                           -- Part of TempusMUD
//

#include <string.h>
#include <time.h>
#include <tmpstr.h>
#include <ctype.h>
#include <stdlib.h>
#include "interpreter.h"
#include "constants.h"
#include "language.h"
#include "utils.h"
#include "creature.h"
#include "comm.h"
#include "screen.h"
#include "handler.h"

extern const char *player_race[];

const char *language_names[] = {
	"modriatic",				//Past Time Frame
	"elvish",					//Elf
	"dwarven",					//Dwarf
	"orcish",					//Orc
	"klingon",					//Klingon
	"hobbish",					//Halflings
	"sylvan",					//Tabaxi
	"underdark",				//Drow and Duergar
	"mordorian",				//Goblin
	"daedalus",					//Minotaur
	"trollish",					//Troll
	"ogrish",					//Ogre
	"infernum",					//Infernum
	"trogish",					//Trog
	"manticora",				//Manticore
	"ghennish",					//Bugbear
	"draconian",				//Dragon and Draconian
	"inconnu",					//Alien 1 and Predator Alien
	"abyssal",					//Demon and Daemon
	"astral",					//Deva, slaad
	"sibilant",					//Illithid
	"gish",						//Githyanki
	"centralian",				//Future Time Frame
	"kobolas",					//Kobold
	"kalerrian",				//Wemic
	"rakshasian",				//Rakshasa
	"gryphus",					//Griffin
	"rotarial",					//Rotarian
	"celestial",				//Archons, Celestial, Gods
	"elysian",					//Guardinal 
	"greek",					//Language of Olypus
	"deadite",					//Undead
	"elemental",				//Elemental Creatures
	"\n"
};

const char *race_language[][2] = {
	{"Human", "modriatic"},
	{"Elf", "elvish"},
	{"Dwarf", "dwarven"},
	{"Half Orc", "orcish"},
	{"Klingon", "klingon"},
	{"Halfling", "hobbish"},
	{"Tabaxi", "sylvan"},
	{"Drow", "underdark"},
	{"ILL", ""},
	{"ILL", ""},
	{"Mobile", ""},
	{"Undead", "deadite"},
	{"Humanoid", ""},
	{"Animal", ""},
	{"Dragon", "draconian"},
	{"Giant", "ogrish"},
	{"Orc", "orcish"},
	{"Goblin", "mordorian"},
	{"Hafling", "hobbish"},
	{"Minotaur", "daedalus"},
	{"Troll", "trollish"},
	{"Golem", ""},
	{"Elemental", "elemental"},
	{"Ogre", "ogrish"},
	{"Devil", "infernum"},
	{"Trog", "trogish"},
	{"Manticore", "manticora"},
	{"Bugbear", "ghennish"},
	{"Draconian", "draconian"},
	{"Duergar", "underdark"},
	{"Slaad", "astral"},
	{"Robot", "centralian"},
	{"Demon", "abyssal"},
	{"Deva", "abyssal"},
	{"Plant", ""},
	{"Archon", "celestial"},
	{"Pudding", ""},
	{"Alien 1", "inconnu"},
	{"Predator Alien", "inconnu"},
	{"Slime", ""},
	{"Illithid", "sibilant"},
	{"Fish", ""},
	{"Beholder", "underdark"},
	{"Gaseous", "elemental"},
	{"Githyanki", "gish"},
	{"Insect", ""},
	{"Daemon", "abyssal"},
	{"Mephit", "elemental"},
	{"Kobold", "kobolas"},
	{"Umber Hulk", "underdark"},
	{"Wemic", "kalerrian"},
	{"Rakshasa", "rakshasian"},
	{"Spider", ""},
	{"Griffin", "gryphus"},
	{"Rotarian", "rotarial"},
	{"Half Elf", "elvish"},
	{"Celestial", "celestial"},
	{"Guardinal", "elysian"},
	{"Olympian", "greek"},
	{"Yugoloth", "all"},
	{"Rowlahr", ""},
	{"Githzerai", "gish"},
	{"\n", "\n"},
};

struct syl_struct languages[] = {
// Human
	{"the", "ute"},
	{"and", "uhauo"},
	{"for", "ovhi"},
	{"he", "exu"},
	{"she", "yoby"},
	{"now", "o'oga"},
	{"but", "iru"},
	{"my", "ire"},
	{"it", "aly"},
	{"you", "ecese"},
	{"me", "viot"},
	{"has", "rhun"},
	{"well", "ehe"},
	{"are", "afozi"},
	{"what", "equlog"},
	{"in", "up"},
	{"an", "sis"},
	{"-", "yly"},
	{"this", "abhu"},
	{"that", "uwyob"},
	{"there", "'qu"},
	{"their", "ofu"},
	{"ask", "locae"},
	{"where", "gyanik"},
	{"how", "yaq"},
	{"is", "iu"},
	{"they", "uve"},
	{"they're", "ufl"},
	{"can", "e'u"},
	{"on", "goiz"},
	{"or", "yep"},
	{"while", "exyeg"},
	{"b", "c"}, {"c", "t"}, {"d", "x"}, {"f", "n"}, {"g", "v"},
	{"h", "p"}, {"j", "q"}, {"k", "b"}, {"l", "s"}, {"m", "z"},
	{"n", "g"}, {"p", "m"}, {"q", "w"}, {"r", "h"}, {"s", "j"},
	{"t", "r"}, {"v", "l"}, {"w", "k"}, {"x", "y"}, {"y", "f"},
	{"z", "d"}, {"a", "e"}, {"e", "a"}, {"i", "u"}, {"o", "i"},
	{"u", "o"},
// Elvish
	{"the", "he"},
	{"and", "ahi"},
	{"for", "wi"},
	{"he", "momi"},
	{"she", "akok"},
	{"now", "uba"},
	{"but", "abaj"},
	{"my", "ileg"},
	{"it", "am"},
	{"you", "pu"},
	{"me", "asl"},
	{"has", "ucozi"},
	{"well", "ej"},
	{"are", "ibo"},
	{"what", "imi"},
	{"in", "liq"},
	{"an", "ugeg"},
	{"-", "abr"},
	{"this", "ubivy"},
	{"that", "her"},
	{"there", "aba"},
	{"their", "uxobiti"},
	{"ask", "iho"},
	{"where", "hacte"},
	{"how", "my"},
	{"is", "owr"},
	{"they", "uo"},
	{"they're", "uqujus"},
	{"can", "qu"},
	{"on", "omab"},
	{"or", "ojt"},
	{"while", "irixoje"},
	{"b", "y"}, {"c", "x"}, {"d", "k"}, {"f", "v"}, {"g", "m"},
	{"h", "d"}, {"j", "s"}, {"k", "z"}, {"l", "t"}, {"m", "h"},
	{"n", "q"}, {"p", "c"}, {"q", "b"}, {"r", "g"}, {"s", "f"},
	{"t", "j"}, {"v", "p"}, {"w", "l"}, {"x", "n"}, {"y", "w"},
	{"z", "r"}, {"a", "i"}, {"e", "o"}, {"i", "e"}, {"o", "a"},
	{"u", "u"},
// Dwarvish
	{"the", "a'a"},
	{"and", "vyr"},
	{"for", "ovi"},
	{"he", "izeh"},
	{"she", "rugo"},
	{"now", "ioq"},
	{"but", "dylu"},
	{"my", "ier"},
	{"it", "te'"},
	{"you", "hyo"},
	{"me", "dlo"},
	{"has", "oso"},
	{"well", "wya"},
	{"are", "alaco"},
	{"what", "ha"},
	{"in", "oe'i"},
	{"an", "lip"},
	{"-", "yox"},
	{"this", "utagh"},
	{"that", "uwi"},
	{"there", "yl"},
	{"their", "rli"},
	{"ask", "ud"},
	{"where", "lish"},
	{"how", "auvu"},
	{"is", "paj"},
	{"they", "apl"},
	{"they're", "laohoqur"},
	{"can", "unewh"},
	{"on", "cot"},
	{"or", "arho"},
	{"while", "aki"},
	{"b", "s"}, {"c", "g"}, {"d", "c"}, {"f", "m"}, {"g", "j"},
	{"h", "b"}, {"j", "l"}, {"k", "r"}, {"l", "v"}, {"m", "x"},
	{"n", "q"}, {"p", "t"}, {"q", "y"}, {"r", "f"}, {"s", "k"},
	{"t", "w"}, {"v", "h"}, {"w", "n"}, {"x", "z"}, {"y", "p"},
	{"z", "d"}, {"a", "u"}, {"e", "a"}, {"i", "o"}, {"o", "e"},
	{"u", "i"},
// Orcish
	{"the", "bigol"},
	{"and", "pus"},
	{"for", "eik"},
	{"he", "ur"},
	{"she", "lig"},
	{"now", "yl"},
	{"but", "yoel"},
	{"my", "as"},
	{"it", "ua'r"},
	{"you", "otha"},
	{"me", "oxi"},
	{"has", "ozlu"},
	{"well", "diryuw"},
	{"are", "enon"},
	{"what", "oph"},
	{"in", "ham"},
	{"an", "oes"},
	{"-", "ab"},
	{"this", "rya"},
	{"that", "rajema"},
	{"there", "roz"},
	{"their", "eosuaty"},
	{"ask", "ese"},
	{"where", "wyryair"},
	{"how", "ha"},
	{"is", "azo"},
	{"they", "blahl"},
	{"they're", "metyez"},
	{"can", "neh"},
	{"on", "ipi"},
	{"or", "oma"},
	{"while", "ol"},
	{"b", "f"}, {"c", "s"}, {"d", "t"}, {"f", "r"}, {"g", "z"},
	{"h", "k"}, {"j", "w"}, {"k", "d"}, {"l", "c"}, {"m", "h"},
	{"n", "x"}, {"p", "m"}, {"q", "v"}, {"r", "y"}, {"s", "p"},
	{"t", "b"}, {"v", "n"}, {"w", "l"}, {"x", "q"}, {"y", "g"},
	{"z", "j"}, {"a", "o"}, {"e", "u"}, {"i", "e"}, {"o", "a"},
	{"u", "i"},
// Klingon
	{"the", "ubu"},
	{"and", "ohu"},
	{"for", "lizr"},
	{"he", "ev"},
	{"she", "oth"},
	{"now", "of"},
	{"but", "go"},
	{"my", "yli"},
	{"it", "umug"},
	{"you", "ijo"},
	{"me", "uj"},
	{"has", "azizy"},
	{"well", "erl"},
	{"are", "lyup"},
	{"what", "agaja"},
	{"in", "syob"},
	{"an", "uso"},
	{"-", "li"},
	{"this", "ualyl"},
	{"that", "wimoa"},
	{"there", "uqup"},
	{"their", "ob"},
	{"ask", "odoid"},
	{"where", "mujhly"},
	{"how", "ewy"},
	{"is", "hly"},
	{"they", "roplo"},
	{"they're", "awrlechly"},
	{"can", "ij"},
	{"on", "ryut"},
	{"or", "'yr"},
	{"while", "hafy"},
	{"b", "t"}, {"c", "g"}, {"d", "b"}, {"f", "j"}, {"g", "q"},
	{"h", "m"}, {"j", "f"}, {"k", "d"}, {"l", "c"}, {"m", "h"},
	{"n", "y"}, {"p", "w"}, {"q", "v"}, {"r", "k"}, {"s", "x"},
	{"t", "p"}, {"v", "s"}, {"w", "r"}, {"x", "l"}, {"y", "n"},
	{"z", "z"}, {"a", "e"}, {"e", "u"}, {"i", "a"}, {"o", "i"},
	{"u", "o"},
// Halfling
	{"the", "hity"},
	{"and", "'ugeo"},
	{"for", "oriro"},
	{"he", "emya"},
	{"she", "usyo"},
	{"now", "dup"},
	{"but", "qu"},
	{"my", "quro"},
	{"it", "udyl"},
	{"you", "ni"},
	{"me", "ug"},
	{"has", "laqu"},
	{"well", "e'd"},
	{"are", "jhuta"},
	{"what", "ulyia"},
	{"in", "hua"},
	{"an", "hy"},
	{"-", "ehu"},
	{"this", "of"},
	{"that", "tizu"},
	{"there", "yacafib"},
	{"their", "ifou'"},
	{"ask", "iwe"},
	{"where", "ekio"},
	{"how", "huge"},
	{"is", "ludh"},
	{"they", "li"},
	{"they're", "ikt"},
	{"can", "ovo'u"},
	{"on", "uzuf"},
	{"or", "gy"},
	{"while", "utiutry"},
	{"b", "j"}, {"c", "y"}, {"d", "v"}, {"f", "k"}, {"g", "n"},
	{"h", "m"}, {"j", "p"}, {"k", "g"}, {"l", "f"}, {"m", "d"},
	{"n", "s"}, {"p", "z"}, {"q", "x"}, {"r", "w"}, {"s", "l"},
	{"t", "q"}, {"v", "r"}, {"w", "t"}, {"x", "h"}, {"y", "b"},
	{"z", "c"}, {"a", "e"}, {"e", "i"}, {"i", "a"}, {"o", "u"},
	{"u", "o"},
// Tabaxi
	{"the", "uci"},
	{"and", "them"},
	{"for", "vuaru"},
	{"he", "eja"},
	{"she", "ab"},
	{"now", "ocidu"},
	{"but", "oiare"},
	{"my", "eref"},
	{"it", "eduk"},
	{"you", "ro"},
	{"me", "lyr"},
	{"has", "e'"},
	{"well", "awrio"},
	{"are", "afi"},
	{"what", "heoes"},
	{"in", "loiu"},
	{"an", "ek"},
	{"-", "tr"},
	{"this", "iwil"},
	{"that", "gaeici"},
	{"there", "liale"},
	{"their", "yijt"},
	{"ask", "li"},
	{"where", "jy"},
	{"how", "olyrl"},
	{"is", "iji"},
	{"they", "iezrl"},
	{"they're", "equvitri'"},
	{"can", "vlej"},
	{"on", "le"},
	{"or", "re"},
	{"while", "ryuqu"},
	{"b", "m"}, {"c", "j"}, {"d", "b"}, {"f", "t"}, {"g", "c"},
	{"h", "f"}, {"j", "n"}, {"k", "x"}, {"l", "z"}, {"m", "r"},
	{"n", "s"}, {"p", "w"}, {"q", "g"}, {"r", "k"}, {"s", "h"},
	{"t", "v"}, {"v", "l"}, {"w", "d"}, {"x", "q"}, {"y", "p"},
	{"z", "y"}, {"a", "o"}, {"e", "u"}, {"i", "e"}, {"o", "a"},
	{"u", "i"},
// Drowish
	{"the", "oce"},
	{"and", "usid"},
	{"for", "imi"},
	{"he", "oqu"},
	{"she", "ikyo"},
	{"now", "uevo"},
	{"but", "vh"},
	{"my", "eg"},
	{"it", "hun"},
	{"you", "anoao"},
	{"me", "yuib"},
	{"has", "us"},
	{"well", "iha"},
	{"are", "orif"},
	{"what", "oririv"},
	{"in", "yr"},
	{"an", "ubr"},
	{"-", "i'h"},
	{"this", "e'o"},
	{"that", "um"},
	{"there", "ola"},
	{"their", "iua"},
	{"ask", "kyo"},
	{"where", "uosh"},
	{"how", "akyik"},
	{"is", "oqu"},
	{"they", "ov"},
	{"they're", "okovyl"},
	{"can", "jye"},
	{"on", "ha"},
	{"or", "axy"},
	{"while", "ofexru"},
	{"b", "y"}, {"c", "r"}, {"d", "b"}, {"f", "z"}, {"g", "c"},
	{"h", "s"}, {"j", "x"}, {"k", "v"}, {"l", "g"}, {"m", "t"},
	{"n", "d"}, {"p", "j"}, {"q", "k"}, {"r", "l"}, {"s", "n"},
	{"t", "m"}, {"v", "f"}, {"w", "q"}, {"x", "w"}, {"y", "h"},
	{"z", "p"}, {"a", "e"}, {"e", "o"}, {"i", "a"}, {"o", "i"},
	{"u", "u"},
// Goblin
	{"the", "ro'lo"},
	{"and", "tyr"},
	{"for", "ogy"},
	{"he", "rle"},
	{"she", "loj"},
	{"now", "ro"},
	{"but", "rehip"},
	{"my", "hyi"},
	{"it", "uiz"},
	{"you", "oeor"},
	{"me", "apa"},
	{"has", "ru'l"},
	{"well", "yasige"},
	{"are", "etlei"},
	{"what", "osa"},
	{"in", "yr"},
	{"an", "ixif"},
	{"-", "ktl"},
	{"this", "elica"},
	{"that", "oas"},
	{"there", "laweg"},
	{"their", "afaihi"},
	{"ask", "ig"},
	{"where", "wyruxi"},
	{"how", "ed"},
	{"is", "ezo"},
	{"they", "ewrh"},
	{"they're", "akudl"},
	{"can", "yei"},
	{"on", "yal"},
	{"or", "hiw"},
	{"while", "lozy"},
	{"b", "f"}, {"c", "g"}, {"d", "t"}, {"f", "c"}, {"g", "j"},
	{"h", "v"}, {"j", "z"}, {"k", "x"}, {"l", "s"}, {"m", "h"},
	{"n", "p"}, {"p", "k"}, {"q", "l"}, {"r", "n"}, {"s", "w"},
	{"t", "d"}, {"v", "y"}, {"w", "q"}, {"x", "r"}, {"y", "m"},
	{"z", "b"}, {"a", "o"}, {"e", "i"}, {"i", "e"}, {"o", "a"},
	{"u", "u"},
// Minotaur
	{"the", "oli"},
	{"and", "mi'"},
	{"for", "ylua"},
	{"he", "iuxi"},
	{"she", "wiod"},
	{"now", "exiwe"},
	{"but", "le"},
	{"my", "ovu"},
	{"it", "ecy"},
	{"you", "oez"},
	{"me", "aka"},
	{"has", "wru"},
	{"well", "hite"},
	{"are", "ejejt"},
	{"what", "ita"},
	{"in", "unu"},
	{"an", "pa"},
	{"-", "gh"},
	{"this", "dajo"},
	{"that", "hevlis"},
	{"there", "uluor"},
	{"their", "yuo"},
	{"ask", "wesla"},
	{"where", "odumu"},
	{"how", "iu"},
	{"is", "ebr"},
	{"they", "li"},
	{"they're", "ugomuj"},
	{"can", "alu"},
	{"on", "eaka"},
	{"or", "afe"},
	{"while", "aew"},
	{"b", "d"}, {"c", "y"}, {"d", "v"}, {"f", "x"}, {"g", "z"},
	{"h", "r"}, {"j", "f"}, {"k", "q"}, {"l", "j"}, {"m", "g"},
	{"n", "w"}, {"p", "k"}, {"q", "n"}, {"r", "s"}, {"s", "h"},
	{"t", "b"}, {"v", "p"}, {"w", "t"}, {"x", "l"}, {"y", "m"},
	{"z", "c"}, {"a", "i"}, {"e", "o"}, {"i", "u"}, {"o", "e"},
	{"u", "a"},
// Trollish
	{"the", "jady"},
	{"and", "ye"},
	{"for", "ziot"},
	{"he", "o'"},
	{"she", "quz"},
	{"now", "ag"},
	{"but", "uw"},
	{"my", "ciz"},
	{"it", "yi"},
	{"you", "uevhy"},
	{"me", "azho"},
	{"has", "ylyo"},
	{"well", "hazwo"},
	{"are", "opo"},
	{"what", "hy"},
	{"in", "ur"},
	{"an", "iqu"},
	{"-", "gh"},
	{"this", "yrio"},
	{"that", "os"},
	{"there", "limy"},
	{"their", "ofli"},
	{"ask", "acoxr"},
	{"where", "ociqu"},
	{"how", "lufij"},
	{"is", "ruik"},
	{"they", "ui"},
	{"they're", "quwefl"},
	{"can", "e'to"},
	{"on", "uikt"},
	{"or", "yer"},
	{"while", "eguquj"},
	{"b", "q"}, {"c", "l"}, {"d", "g"}, {"f", "m"}, {"g", "r"},
	{"h", "s"}, {"j", "c"}, {"k", "f"}, {"l", "t"}, {"m", "b"},
	{"n", "y"}, {"p", "n"}, {"q", "j"}, {"r", "d"}, {"s", "k"},
	{"t", "h"}, {"v", "p"}, {"w", "z"}, {"x", "v"}, {"y", "x"},
	{"z", "w"}, {"a", "i"}, {"e", "o"}, {"i", "a"}, {"o", "u"},
	{"u", "e"},
// Ogre
	{"the", "avl"},
	{"and", "apo"},
	{"for", "lyuw"},
	{"he", "ucoz"},
	{"she", "yox"},
	{"now", "uluki"},
	{"but", "lewy"},
	{"my", "ino"},
	{"it", "bac"},
	{"you", "a'e"},
	{"me", "uci"},
	{"has", "dhlof"},
	{"well", "ojo"},
	{"are", "trup"},
	{"what", "se'"},
	{"in", "um"},
	{"an", "riha"},
	{"-", "iny"},
	{"this", "ylany"},
	{"that", "olac"},
	{"there", "aeijtyo"},
	{"their", "ao"},
	{"ask", "iuk"},
	{"where", "ik"},
	{"how", "elo"},
	{"is", "yud"},
	{"they", "oubo"},
	{"they're", "u'ubres"},
	{"can", "ale"},
	{"on", "yofa"},
	{"or", "rez"},
	{"while", "aflama"},
	{"b", "t"}, {"c", "n"}, {"d", "r"}, {"f", "g"}, {"g", "f"},
	{"h", "l"}, {"j", "y"}, {"k", "s"}, {"l", "h"}, {"m", "k"},
	{"n", "v"}, {"p", "d"}, {"q", "x"}, {"r", "m"}, {"s", "q"},
	{"t", "z"}, {"v", "j"}, {"w", "b"}, {"x", "c"}, {"y", "p"},
	{"z", "w"}, {"a", "e"}, {"e", "u"}, {"i", "o"}, {"o", "a"},
	{"u", "i"},
// Devil
	{"the", "oty"},
	{"and", "uch"},
	{"for", "ur"},
	{"he", "oleu"},
	{"she", "leafh"},
	{"now", "elo"},
	{"but", "yry"},
	{"my", "oea"},
	{"it", "my"},
	{"you", "'eote"},
	{"me", "'ol"},
	{"has", "imo"},
	{"well", "evh"},
	{"are", "yaseu"},
	{"what", "agogy"},
	{"in", "heu"},
	{"an", "rye"},
	{"-", "ye"},
	{"this", "lylapi"},
	{"that", "ukan"},
	{"there", "iho"},
	{"their", "uaosem"},
	{"ask", "hau"},
	{"where", "ou"},
	{"how", "one"},
	{"is", "jti'"},
	{"they", "mura"},
	{"they're", "eg"},
	{"can", "fem"},
	{"on", "ov"},
	{"or", "ufu"},
	{"while", "upyilyi"},
	{"b", "w"}, {"c", "m"}, {"d", "l"}, {"f", "c"}, {"g", "t"},
	{"h", "z"}, {"j", "h"}, {"k", "p"}, {"l", "r"}, {"m", "b"},
	{"n", "k"}, {"p", "q"}, {"q", "s"}, {"r", "g"}, {"s", "f"},
	{"t", "x"}, {"v", "n"}, {"w", "d"}, {"x", "v"}, {"y", "j"},
	{"z", "y"}, {"a", "u"}, {"e", "o"}, {"i", "e"}, {"o", "a"},
	{"u", "i"},
// Trogish
	{"the", "yi'"},
	{"and", "eol"},
	{"for", "ihot"},
	{"he", "umo"},
	{"she", "yu"},
	{"now", "yue"},
	{"but", "igh"},
	{"my", "ozy"},
	{"it", "rla"},
	{"you", "adesy"},
	{"me", "shok"},
	{"has", "avy"},
	{"well", "zi"},
	{"are", "uosur"},
	{"what", "omi"},
	{"in", "vo"},
	{"an", "uto"},
	{"-", "ig"},
	{"this", "aoghoa"},
	{"that", "i'"},
	{"there", "'eveux"},
	{"their", "url"},
	{"ask", "ixeze"},
	{"where", "udoi"},
	{"how", "pe"},
	{"is", "cezw"},
	{"they", "eat"},
	{"they're", "rhaq"},
	{"can", "gi"},
	{"on", "ume"},
	{"or", "lyop"},
	{"while", "biplau"},
	{"b", "r"}, {"c", "w"}, {"d", "c"}, {"f", "g"}, {"g", "t"},
	{"h", "d"}, {"j", "h"}, {"k", "j"}, {"l", "k"}, {"m", "z"},
	{"n", "p"}, {"p", "v"}, {"q", "s"}, {"r", "l"}, {"s", "f"},
	{"t", "b"}, {"v", "n"}, {"w", "y"}, {"x", "q"}, {"y", "x"},
	{"z", "m"}, {"a", "o"}, {"e", "a"}, {"i", "u"}, {"o", "i"},
	{"u", "e"},
// Manticora
	{"the", "waje"},
	{"and", "usa"},
	{"for", "efol"},
	{"he", "fla"},
	{"she", "uton"},
	{"now", "oat"},
	{"but", "ati"},
	{"my", "apr"},
	{"it", "hid"},
	{"you", "o'u"},
	{"me", "uogi"},
	{"has", "ipl"},
	{"well", "hoa"},
	{"are", "rhat"},
	{"what", "lu"},
	{"in", "i'y"},
	{"an", "oer"},
	{"-", "ep"},
	{"this", "unul"},
	{"that", "aocio"},
	{"there", "hecexo"},
	{"their", "ulom"},
	{"ask", "ajta"},
	{"where", "leji"},
	{"how", "ol"},
	{"is", "ry"},
	{"they", "ha"},
	{"they're", "wawahe"},
	{"can", "oja"},
	{"on", "eo'r"},
	{"or", "yle"},
	{"while", "esuceg"},
	{"b", "c"}, {"c", "r"}, {"d", "q"}, {"f", "z"}, {"g", "d"},
	{"h", "v"}, {"j", "m"}, {"k", "f"}, {"l", "h"}, {"m", "b"},
	{"n", "t"}, {"p", "n"}, {"q", "x"}, {"r", "p"}, {"s", "w"},
	{"t", "s"}, {"v", "k"}, {"w", "g"}, {"x", "y"}, {"y", "j"},
	{"z", "l"}, {"a", "i"}, {"e", "o"}, {"i", "a"}, {"o", "u"},
	{"u", "e"},
// Bugbear
	{"the", "lip"},
	{"and", "yia"},
	{"for", "ekth"},
	{"he", "ela"},
	{"she", "lyl"},
	{"now", "oqusy"},
	{"but", "ha"},
	{"my", "yi"},
	{"it", "i'ye"},
	{"you", "oju"},
	{"me", "ie"},
	{"has", "lewr"},
	{"well", "aphlyu"},
	{"are", "evha"},
	{"what", "ur"},
	{"in", "odla"},
	{"an", "jofy"},
	{"-", "aro"},
	{"this", "hiat"},
	{"that", "hepuk"},
	{"there", "kt"},
	{"their", "uphepeh"},
	{"ask", "na"},
	{"where", "uce"},
	{"how", "qup"},
	{"is", "uwh"},
	{"they", "aw"},
	{"they're", "pyet"},
	{"can", "ifh"},
	{"on", "lixo"},
	{"or", "ujoc"},
	{"while", "uotle"},
	{"b", "r"}, {"c", "f"}, {"d", "x"}, {"f", "m"}, {"g", "w"},
	{"h", "v"}, {"j", "c"}, {"k", "q"}, {"l", "z"}, {"m", "y"},
	{"n", "s"}, {"p", "n"}, {"q", "g"}, {"r", "h"}, {"s", "l"},
	{"t", "j"}, {"v", "t"}, {"w", "p"}, {"x", "b"}, {"y", "d"},
	{"z", "k"}, {"a", "o"}, {"e", "i"}, {"i", "u"}, {"o", "a"},
	{"u", "e"},
// Draconian
	{"the", "eq"},
	{"and", "tad"},
	{"for", "ema"},
	{"he", "umab"},
	{"she", "aoz"},
	{"now", "lu"},
	{"but", "ime"},
	{"my", "yi"},
	{"it", "he'l"},
	{"you", "yliam"},
	{"me", "tik"},
	{"has", "eh"},
	{"well", "'o"},
	{"are", "tlo"},
	{"what", "ovy"},
	{"in", "yr"},
	{"an", "oxar"},
	{"-", "yih"},
	{"this", "vipe"},
	{"that", "oke"},
	{"there", "epl'izhe"},
	{"their", "ya"},
	{"ask", "esaqu"},
	{"where", "oladiur"},
	{"how", "ep"},
	{"is", "ovu"},
	{"they", "ufug"},
	{"they're", "yokto"},
	{"can", "ejelo"},
	{"on", "ra"},
	{"or", "efod"},
	{"while", "yuox"},
	{"b", "m"}, {"c", "x"}, {"d", "q"}, {"f", "r"}, {"g", "d"},
	{"h", "k"}, {"j", "f"}, {"k", "h"}, {"l", "s"}, {"m", "y"},
	{"n", "z"}, {"p", "j"}, {"q", "w"}, {"r", "v"}, {"s", "p"},
	{"t", "g"}, {"v", "n"}, {"w", "l"}, {"x", "c"}, {"y", "t"},
	{"z", "b"}, {"a", "u"}, {"e", "i"}, {"i", "o"}, {"o", "a"},
	{"u", "e"},
// Duergar
	{"the", "onu"},
	{"and", "uamu"},
	{"for", "ipofy"},
	{"he", "hu"},
	{"she", "jye"},
	{"now", "yir"},
	{"but", "fesa"},
	{"my", "uxaz"},
	{"it", "loxo"},
	{"you", "eube"},
	{"me", "ose"},
	{"has", "oeg"},
	{"well", "boe"},
	{"are", "ux"},
	{"what", "ege"},
	{"in", "uvi"},
	{"an", "ulu"},
	{"-", "apy"},
	{"this", "ibhyua"},
	{"that", "quluw"},
	{"there", "ubu"},
	{"their", "iri"},
	{"ask", "oqu'o"},
	{"where", "actevuh"},
	{"how", "prl"},
	{"is", "ape"},
	{"they", "uzw"},
	{"they're", "lob"},
	{"can", "ribus"},
	{"on", "iso"},
	{"or", "ef"},
	{"while", "yrh"},
	{"b", "x"}, {"c", "y"}, {"d", "z"}, {"f", "w"}, {"g", "n"},
	{"h", "k"}, {"j", "f"}, {"k", "l"}, {"l", "d"}, {"m", "h"},
	{"n", "r"}, {"p", "m"}, {"q", "p"}, {"r", "v"}, {"s", "g"},
	{"t", "s"}, {"v", "c"}, {"w", "q"}, {"x", "j"}, {"y", "b"},
	{"z", "t"}, {"a", "i"}, {"e", "o"}, {"i", "e"}, {"o", "a"},
	{"u", "u"},
// Demon
	{"the", "dat"},
	{"and", "oasik"},
	{"for", "yle"},
	{"he", "ev"},
	{"she", "ali"},
	{"now", "ai'"},
	{"but", "iwr"},
	{"my", "yov"},
	{"it", "aku"},
	{"you", "isl"},
	{"me", "yl"},
	{"has", "ae"},
	{"well", "oiagy"},
	{"are", "led"},
	{"what", "uef"},
	{"in", "a'"},
	{"an", "ekt"},
	{"-", "ur"},
	{"this", "if"},
	{"that", "bakt"},
	{"there", "uli"},
	{"their", "riz"},
	{"ask", "'lo"},
	{"where", "uh"},
	{"how", "iev"},
	{"is", "u'o"},
	{"they", "e'o'"},
	{"they're", "ivy"},
	{"can", "ai"},
	{"on", "orlu"},
	{"or", "ep"},
	{"while", "abaebo"},
	{"b", "r"}, {"c", "p"}, {"d", "j"}, {"f", "x"}, {"g", "z"},
	{"h", "y"}, {"j", "l"}, {"k", "c"}, {"l", "b"}, {"m", "h"},
	{"n", "g"}, {"p", "s"}, {"q", "m"}, {"r", "d"}, {"s", "f"},
	{"t", "q"}, {"v", "k"}, {"w", "n"}, {"x", "v"}, {"y", "t"},
	{"z", "w"}, {"a", "u"}, {"e", "a"}, {"i", "e"}, {"o", "i"},
	{"u", "o"},
// Deva
	{"the", "uw"},
	{"and", "ohi"},
	{"for", "eseu"},
	{"he", "xu'"},
	{"she", "opr"},
	{"now", "itl"},
	{"but", "eij"},
	{"my", "a'u"},
	{"it", "ujiu"},
	{"you", "yia"},
	{"me", "oteo"},
	{"has", "umore"},
	{"well", "uc"},
	{"are", "e'to'"},
	{"what", "eri'"},
	{"in", "plaq"},
	{"an", "fe'i"},
	{"-", "o't"},
	{"this", "ibrho"},
	{"that", "a'hos"},
	{"there", "iur"},
	{"their", "havhij"},
	{"ask", "elu"},
	{"where", "oesl"},
	{"how", "yl"},
	{"is", "yuos"},
	{"they", "aqumai"},
	{"they're", "eaz"},
	{"can", "oh"},
	{"on", "yur"},
	{"or", "lel"},
	{"while", "avoq"},
	{"b", "s"}, {"c", "x"}, {"d", "m"}, {"f", "t"}, {"g", "r"},
	{"h", "b"}, {"j", "c"}, {"k", "j"}, {"l", "z"}, {"m", "k"},
	{"n", "w"}, {"p", "f"}, {"q", "g"}, {"r", "p"}, {"s", "y"},
	{"t", "d"}, {"v", "n"}, {"w", "h"}, {"x", "q"}, {"y", "v"},
	{"z", "l"}, {"a", "o"}, {"e", "a"}, {"i", "e"}, {"o", "i"},
	{"u", "u"},
// Illithid
	{"the", "yequ"},
	{"and", "eci"},
	{"for", "mear"},
	{"he", "yet"},
	{"she", "uta"},
	{"now", "ashom"},
	{"but", "ehu"},
	{"my", "e'f"},
	{"it", "fly"},
	{"you", "hl"},
	{"me", "helo"},
	{"has", "hef"},
	{"well", "adh"},
	{"are", "roply"},
	{"what", "odli"},
	{"in", "lubl"},
	{"an", "a'wi"},
	{"-", "ezl"},
	{"this", "haiso"},
	{"that", "hon"},
	{"there", "ica"},
	{"their", "iaup"},
	{"ask", "aj"},
	{"where", "enumali"},
	{"how", "hoj"},
	{"is", "zla"},
	{"they", "ninocy"},
	{"they're", "ubyom"},
	{"can", "ebyes"},
	{"on", "ithu"},
	{"or", "hl"},
	{"while", "ku'"},
	{"b", "w"}, {"c", "v"}, {"d", "x"}, {"f", "y"}, {"g", "z"},
	{"h", "m"}, {"j", "s"}, {"k", "n"}, {"l", "r"}, {"m", "k"},
	{"n", "g"}, {"p", "d"}, {"q", "c"}, {"r", "j"}, {"s", "q"},
	{"t", "h"}, {"v", "l"}, {"w", "f"}, {"x", "b"}, {"y", "p"},
	{"z", "t"}, {"a", "i"}, {"e", "a"}, {"i", "e"}, {"o", "u"},
	{"u", "o"},
// Githyanki
	{"the", "ryrev"},
	{"and", "hie"},
	{"for", "hom"},
	{"he", "it"},
	{"she", "hopy"},
	{"now", "avy"},
	{"but", "itr"},
	{"my", "ib"},
	{"it", "reg"},
	{"you", "emy"},
	{"me", "om"},
	{"has", "yuquo"},
	{"well", "ruf"},
	{"are", "irix"},
	{"what", "avh"},
	{"in", "uz"},
	{"an", "yl"},
	{"-", "aqu"},
	{"this", "isl"},
	{"that", "yiwoan"},
	{"there", "elu"},
	{"their", "asel"},
	{"ask", "ec"},
	{"where", "tl"},
	{"how", "awrhu"},
	{"is", "ai"},
	{"they", "u'l"},
	{"they're", "hy"},
	{"can", "whu"},
	{"on", "uha"},
	{"or", "em"},
	{"while", "ek"},
	{"b", "d"}, {"c", "p"}, {"d", "y"}, {"f", "z"}, {"g", "q"},
	{"h", "m"}, {"j", "g"}, {"k", "h"}, {"l", "x"}, {"m", "n"},
	{"n", "s"}, {"p", "k"}, {"q", "w"}, {"r", "c"}, {"s", "j"},
	{"t", "l"}, {"v", "r"}, {"w", "t"}, {"x", "f"}, {"y", "v"},
	{"z", "b"}, {"a", "o"}, {"e", "a"}, {"i", "e"}, {"o", "i"},
	{"u", "u"},
// Daemon
	{"the", "lin"},
	{"and", "'om"},
	{"for", "use'"},
	{"he", "su"},
	{"she", "auv"},
	{"now", "hu"},
	{"but", "ratl"},
	{"my", "ofi"},
	{"it", "uzug"},
	{"you", "lae"},
	{"me", "uch"},
	{"has", "na"},
	{"well", "ekta"},
	{"are", "e'i'"},
	{"what", "ihyei"},
	{"in", "ly"},
	{"an", "uct"},
	{"-", "hu"},
	{"this", "afya"},
	{"that", "det"},
	{"there", "jawonyr"},
	{"their", "iriceda"},
	{"ask", "yuhy"},
	{"where", "uk"},
	{"how", "vh"},
	{"is", "brl"},
	{"they", "opeha"},
	{"they're", "ologhla"},
	{"can", "eme"},
	{"on", "kta"},
	{"or", "ea"},
	{"while", "cyupa"},
	{"b", "f"}, {"c", "g"}, {"d", "v"}, {"f", "r"}, {"g", "z"},
	{"h", "t"}, {"j", "x"}, {"k", "l"}, {"l", "h"}, {"m", "n"},
	{"n", "b"}, {"p", "j"}, {"q", "s"}, {"r", "c"}, {"s", "y"},
	{"t", "k"}, {"v", "d"}, {"w", "p"}, {"x", "m"}, {"y", "q"},
	{"z", "w"}, {"a", "e"}, {"e", "a"}, {"i", "u"}, {"o", "i"},
	{"u", "o"},
// Kobold
	{"the", "hiz"},
	{"and", "li"},
	{"for", "oply"},
	{"he", "ox"},
	{"she", "jhy"},
	{"now", "umyl"},
	{"but", "hig"},
	{"my", "yi"},
	{"it", "tehl"},
	{"you", "os"},
	{"me", "haz"},
	{"has", "yu"},
	{"well", "iti"},
	{"are", "rec"},
	{"what", "iq"},
	{"in", "up"},
	{"an", "yisi"},
	{"-", "ush"},
	{"this", "ot"},
	{"that", "rhyle"},
	{"there", "yuavic"},
	{"their", "akicuvy"},
	{"ask", "izry"},
	{"where", "yo'er"},
	{"how", "oj"},
	{"is", "eju"},
	{"they", "oh"},
	{"they're", "yrepoqu"},
	{"can", "ixuw"},
	{"on", "yrlu"},
	{"or", "ono"},
	{"while", "locexe"},
	{"b", "c"}, {"c", "s"}, {"d", "f"}, {"f", "x"}, {"g", "z"},
	{"h", "b"}, {"j", "l"}, {"k", "n"}, {"l", "q"}, {"m", "h"},
	{"n", "t"}, {"p", "d"}, {"q", "y"}, {"r", "j"}, {"s", "m"},
	{"t", "p"}, {"v", "w"}, {"w", "v"}, {"x", "g"}, {"y", "r"},
	{"z", "k"}, {"a", "i"}, {"e", "u"}, {"i", "o"}, {"o", "a"},
	{"u", "e"},
// Wemic
	{"the", "aro"},
	{"and", "iek"},
	{"for", "ipro"},
	{"he", "eav"},
	{"she", "liek"},
	{"now", "yez"},
	{"but", "ufyo"},
	{"my", "rok"},
	{"it", "ul"},
	{"you", "ogawi"},
	{"me", "ac"},
	{"has", "isy"},
	{"well", "ule"},
	{"are", "iq"},
	{"what", "oep"},
	{"in", "us"},
	{"an", "ifa"},
	{"-", "ghl"},
	{"this", "unacak"},
	{"that", "ugli"},
	{"there", "her"},
	{"their", "yaxo'"},
	{"ask", "yrlop"},
	{"where", "lyuvum"},
	{"how", "lyit"},
	{"is", "hiza"},
	{"they", "ya"},
	{"they're", "a'luctyax"},
	{"can", "hunu'"},
	{"on", "eio"},
	{"or", "ciq"},
	{"while", "eq"},
	{"b", "k"}, {"c", "p"}, {"d", "l"}, {"f", "m"}, {"g", "v"},
	{"h", "y"}, {"j", "d"}, {"k", "w"}, {"l", "h"}, {"m", "x"},
	{"n", "t"}, {"p", "r"}, {"q", "s"}, {"r", "g"}, {"s", "n"},
	{"t", "q"}, {"v", "f"}, {"w", "b"}, {"x", "c"}, {"y", "z"},
	{"z", "j"}, {"a", "i"}, {"e", "o"}, {"i", "u"}, {"o", "a"},
	{"u", "e"},
// Rakshasa
	{"the", "rako"},
	{"and", "rhavi"},
	{"for", "lufl"},
	{"he", "oxr"},
	{"she", "us"},
	{"now", "ohovo"},
	{"but", "uko"},
	{"my", "'uk"},
	{"it", "le"},
	{"you", "ilikh"},
	{"me", "atl"},
	{"has", "ase"},
	{"well", "jtovhi"},
	{"are", "ape"},
	{"what", "ejetu"},
	{"in", "ahi"},
	{"an", "ojux"},
	{"-", "try"},
	{"this", "ot"},
	{"that", "ubu"},
	{"there", "li'a'"},
	{"their", "umi"},
	{"ask", "ha"},
	{"where", "ramyihy"},
	{"how", "aug"},
	{"is", "gudi"},
	{"they", "et"},
	{"they're", "un"},
	{"can", "yi"},
	{"on", "lya"},
	{"or", "oq"},
	{"while", "izwhumy"},
	{"b", "l"}, {"c", "n"}, {"d", "j"}, {"f", "x"}, {"g", "w"},
	{"h", "z"}, {"j", "c"}, {"k", "g"}, {"l", "v"}, {"m", "b"},
	{"n", "p"}, {"p", "y"}, {"q", "h"}, {"r", "s"}, {"s", "m"},
	{"t", "r"}, {"v", "d"}, {"w", "f"}, {"x", "q"}, {"y", "t"},
	{"z", "k"}, {"a", "o"}, {"e", "i"}, {"i", "u"}, {"o", "a"},
	{"u", "e"},
// Griffin
	{"the", "watl"},
	{"and", "yobon"},
	{"for", "u'i"},
	{"he", "ob"},
	{"she", "yil"},
	{"now", "use"},
	{"but", "hylu"},
	{"my", "ozw"},
	{"it", "ulav"},
	{"you", "iju"},
	{"me", "izhi"},
	{"has", "o'y"},
	{"well", "lehou"},
	{"are", "hityr"},
	{"what", "aw"},
	{"in", "tal"},
	{"an", "afe"},
	{"-", "vaq"},
	{"this", "eca"},
	{"that", "oghac"},
	{"there", "ouj"},
	{"their", "ufo"},
	{"ask", "uhe"},
	{"where", "uqu"},
	{"how", "utyo"},
	{"is", "uwo"},
	{"they", "aj"},
	{"they're", "aguhizeu"},
	{"can", "yij"},
	{"on", "eku"},
	{"or", "yob"},
	{"while", "xid"},
	{"b", "h"}, {"c", "d"}, {"d", "c"}, {"f", "z"}, {"g", "v"},
	{"h", "y"}, {"j", "l"}, {"k", "g"}, {"l", "t"}, {"m", "j"},
	{"n", "x"}, {"p", "m"}, {"q", "k"}, {"r", "f"}, {"s", "b"},
	{"t", "n"}, {"v", "s"}, {"w", "p"}, {"x", "r"}, {"y", "q"},
	{"z", "w"}, {"a", "i"}, {"e", "o"}, {"i", "a"}, {"o", "e"},
	{"u", "u"},
// Rotarian
	{"the", "uxi"},
	{"and", "tujtl"},
	{"for", "yu"},
	{"he", "icu"},
	{"she", "rhy"},
	{"now", "oqu"},
	{"but", "oji"},
	{"my", "ul"},
	{"it", "e'y"},
	{"you", "wa"},
	{"me", "oga"},
	{"has", "el"},
	{"well", "eiudeo"},
	{"are", "adl"},
	{"what", "del"},
	{"in", "xy"},
	{"an", "aw"},
	{"-", "ct"},
	{"this", "zly"},
	{"that", "yed"},
	{"there", "ajt"},
	{"their", "un"},
	{"ask", "'ri"},
	{"where", "aletusy"},
	{"how", "exe"},
	{"is", "if"},
	{"they", "hid"},
	{"they're", "zrl"},
	{"can", "xuph"},
	{"on", "obe"},
	{"or", "dly"},
	{"while", "eapoci"},
	{"b", "d"}, {"c", "m"}, {"d", "s"}, {"f", "b"}, {"g", "y"},
	{"h", "g"}, {"j", "c"}, {"k", "t"}, {"l", "w"}, {"m", "j"},
	{"n", "h"}, {"p", "v"}, {"q", "p"}, {"r", "z"}, {"s", "x"},
	{"t", "r"}, {"v", "k"}, {"w", "q"}, {"x", "n"}, {"y", "l"},
	{"z", "f"}, {"a", "o"}, {"e", "i"}, {"i", "u"}, {"o", "a"},
	{"u", "e"},
// Celestial
	{"the", "lyoix"},
	{"and", "yrh"},
	{"for", "gerat"},
	{"he", "esu"},
	{"she", "of"},
	{"now", "roqu"},
	{"but", "lap"},
	{"my", "us"},
	{"it", "amy"},
	{"you", "odai"},
	{"me", "lu"},
	{"has", "a'i"},
	{"well", "yuqub"},
	{"are", "a'hu"},
	{"what", "eh"},
	{"in", "uwo"},
	{"an", "osij"},
	{"-", "itr"},
	{"this", "jeu"},
	{"that", "o'h"},
	{"there", "iluququ"},
	{"their", "ap"},
	{"ask", "utyua"},
	{"where", "urozix"},
	{"how", "hoeg"},
	{"is", "i'it"},
	{"they", "aolefl"},
	{"they're", "hecewonyi"},
	{"can", "ichu"},
	{"on", "yos"},
	{"or", "qul"},
	{"while", "ujipy"},
	{"b", "s"}, {"c", "l"}, {"d", "r"}, {"f", "z"}, {"g", "f"},
	{"h", "p"}, {"j", "c"}, {"k", "d"}, {"l", "b"}, {"m", "v"},
	{"n", "q"}, {"p", "w"}, {"q", "k"}, {"r", "h"}, {"s", "m"},
	{"t", "g"}, {"v", "t"}, {"w", "y"}, {"x", "j"}, {"y", "n"},
	{"z", "x"}, {"a", "u"}, {"e", "i"}, {"i", "a"}, {"o", "e"},
	{"u", "o"},
// Guardinal
	{"the", "eab"},
	{"and", "is"},
	{"for", "luktr"},
	{"he", "ylo"},
	{"she", "atl"},
	{"now", "ni"},
	{"but", "ecude"},
	{"my", "ajt"},
	{"it", "uruw"},
	{"you", "oh"},
	{"me", "qum"},
	{"has", "lopy"},
	{"well", "huc"},
	{"are", "uot"},
	{"what", "kaz"},
	{"in", "yrez"},
	{"an", "iw"},
	{"-", "af"},
	{"this", "leuq"},
	{"that", "lyi"},
	{"there", "ed"},
	{"their", "hiq"},
	{"ask", "ewo"},
	{"where", "uju"},
	{"how", "ome"},
	{"is", "eph"},
	{"they", "luzyla"},
	{"they're", "yevlenini"},
	{"can", "oadle"},
	{"on", "uefl"},
	{"or", "ime"},
	{"while", "ievh"},
	{"b", "p"}, {"c", "l"}, {"d", "w"}, {"f", "n"}, {"g", "j"},
	{"h", "b"}, {"j", "d"}, {"k", "t"}, {"l", "k"}, {"m", "s"},
	{"n", "g"}, {"p", "x"}, {"q", "z"}, {"r", "f"}, {"s", "c"},
	{"t", "h"}, {"v", "r"}, {"w", "q"}, {"x", "m"}, {"y", "v"},
	{"z", "y"}, {"a", "i"}, {"e", "u"}, {"i", "a"}, {"o", "e"},
	{"u", "o"},
// Olympian
	{"the", "hequh"},
	{"and", "xoj"},
	{"for", "erh"},
	{"he", "opu"},
	{"she", "huih"},
	{"now", "lylaj"},
	{"but", "eo"},
	{"my", "ezhl"},
	{"it", "izhl"},
	{"you", "ruw"},
	{"me", "ox"},
	{"has", "exo"},
	{"well", "ijhif"},
	{"are", "uquz"},
	{"what", "ehyose"},
	{"in", "yodo"},
	{"an", "enus"},
	{"-", "urh"},
	{"this", "eh"},
	{"that", "ezly"},
	{"there", "ija"},
	{"their", "hihylyi"},
	{"ask", "lid"},
	{"where", "ukyre"},
	{"how", "labhl"},
	{"is", "ema"},
	{"they", "rhu"},
	{"they're", "ary"},
	{"can", "'b"},
	{"on", "ify"},
	{"or", "ieuz"},
	{"while", "ukei"},
	{"b", "p"}, {"c", "m"}, {"d", "n"}, {"f", "s"}, {"g", "j"},
	{"h", "k"}, {"j", "r"}, {"k", "h"}, {"l", "c"}, {"m", "l"},
	{"n", "q"}, {"p", "d"}, {"q", "g"}, {"r", "b"}, {"s", "v"},
	{"t", "w"}, {"v", "z"}, {"w", "t"}, {"x", "f"}, {"y", "x"},
	{"z", "y"}, {"a", "i"}, {"e", "o"}, {"i", "e"}, {"o", "u"},
	{"u", "a"},
// Deadite    
	{" ", " "},
	{"the", "a'"},
	{"and", "uph"},
	{"for", "iquo"},
	{"he", "ufe"},
	{"she", "uiuat"},
	{"now", "yre"},
	{"but", "la'yi"},
	{"my", "enel"},
	{"it", "vu"},
	{"you", "huko"},
	{"me", "ij"},
	{"has", "iti"},
	{"well", "iog"},
	{"are", "luav"},
	{"what", "ore"},
	{"in", "ri"},
	{"an", "agh"},
	{"-", "lya"},
	{"this", "te"},
	{"that", "heox"},
	{"there", "jaj"},
	{"their", "re"},
	{"ask", "vha"},
	{"where", "aryict"},
	{"how", "eud"},
	{"is", "fhlu"},
	{"they", "apu"},
	{"they're", "onox"},
	{"can", "uri'"},
	{"on", "hlo"},
	{"or", "fab"},
	{"while", "ajtr"},
	{"b", "d"}, {"c", "h"}, {"d", "c"}, {"f", "w"}, {"g", "v"},
	{"h", "r"}, {"j", "f"}, {"k", "z"}, {"l", "q"}, {"m", "g"},
	{"n", "k"}, {"p", "x"}, {"q", "l"}, {"r", "t"}, {"s", "b"},
	{"t", "n"}, {"v", "j"}, {"w", "s"}, {"x", "m"}, {"y", "p"},
	{"z", "y"}, {"a", "i"}, {"e", "o"}, {"i", "u"}, {"o", "a"},
	{"u", "e"},
// Elemental
	{" ", " "},
	{"the", "yloda"},
	{"and", "ict"},
	{"for", "ipos"},
	{"he", "ot"},
	{"she", "hav"},
	{"now", "rei"},
	{"but", "ime"},
	{"my", "ita"},
	{"it", "hop"},
	{"you", "enu'o"},
	{"me", "fyr"},
	{"has", "u'"},
	{"well", "une"},
	{"are", "omaly"},
	{"what", "tlac"},
	{"in", "ume"},
	{"an", "osh"},
	{"-", "abo"},
	{"this", "vaoha"},
	{"that", "lehi"},
	{"there", "hlobiq"},
	{"their", "bejuou"},
	{"ask", "oip"},
	{"where", "woge'i"},
	{"how", "upo"},
	{"is", "yi"},
	{"they", "uwo"},
	{"they're", "syu"},
	{"can", "yeba"},
	{"on", "yul"},
	{"or", "yoi"},
	{"while", "ocusae"},
	{"b", "j"}, {"c", "d"}, {"d", "f"}, {"f", "t"}, {"g", "s"},
	{"h", "v"}, {"j", "l"}, {"k", "z"}, {"l", "p"}, {"m", "k"},
	{"n", "x"}, {"p", "h"}, {"q", "w"}, {"r", "m"}, {"s", "b"},
	{"t", "q"}, {"v", "g"}, {"w", "y"}, {"x", "r"}, {"y", "n"},
	{"z", "c"}, {"a", "u"}, {"e", "o"}, {"i", "a"}, {"o", "e"},
	{"u", "i"}
};

int
find_language_idx(long long language)
{
	int x;

	if (language == 0)
		return 0;

	for (x = 0; x < NUM_LANGUAGES; x++) {
		if (((long long)1 << x) == language)
			return x;
	}
	return LANGUAGE_NONE;
}

int
find_language_idx_by_name(char *language_name)
{
	for (int x = 0; x < NUM_LANGUAGES; x++) {
		if (isname(language_name, language_names[x]))
			return x;
	}

	return LANGUAGE_NONE;
}

int
find_language_idx_by_race(const char *race_name)
{
	char *language_name = NULL;
	 
	for (int x = 0; *race_language[x][0] != '\n'; x++) {
		if (!strcmp(race_language[x][0], race_name)) {
			language_name = tmp_strdup(race_language[x][1]);
			break;
		}
	}

	return language_name ? find_language_idx_by_name(language_name):0;
}

char *
translate_string(const char *phrase, char language_idx)
{
	int word_length;
	char *arg = NULL, *outbuf = NULL;
	bool was_cap;

    if (!strcmp(phrase, ""))
        return tmp_strdup(phrase);

	if (language_idx == LANGUAGE_COMMON)
		return tmp_strdup(phrase);

	while (*phrase) {
		was_cap = false;
		if (isupper(*phrase))
			was_cap = true;
		arg = tmp_getword(&phrase);
		word_length = strlen(arg);

		translate_word(&arg, language_idx);

		if (was_cap)
			arg[0] = toupper(arg[0]);

		if (outbuf != NULL)
			outbuf = tmp_strcat(outbuf, " ", arg, NULL);
		else
			outbuf = tmp_strcat(arg, NULL);
	}

	return tmp_strdup(outbuf);
}

void
translate_word(char **word, char language_idx)
{
	int length;
	int start_pos, end_pos;
	char *arg;
	bool found = false;

	length = strlen(*word);
	arg = tmp_strdup(*word);

	start_pos = language_idx * NUM_SYLLABLES;
	end_pos = ((language_idx + 1) * NUM_SYLLABLES) - 1;

	for (int x = 0; x < end_pos - start_pos - 25; x++) {
		found = false;
		if (!strcmp(arg, languages[start_pos + x].org)) {
			found = true;
			arg = tmp_strdup(languages[start_pos + x].rep);
			break;
		}

	}

	if (!found) {
		int x;
		start_pos = start_pos + ((NUM_SYLLABLES) - 26);
		for (x = 0; x < length; x++) {
			// This sets y == the start of the character translation table
			// for the current language.  26 is the number of letters
			// in the english langugage.
			if (ispunct(arg[x]))
				continue;
			for (int z = 0; z < 26; z++) {
				if (arg[x] == languages[start_pos + z].org[0]) {
					arg[x] = languages[start_pos + z].rep[0];
					break;
				}
			}
		}
		arg[x] = 0;
	}

	*word = tmp_strdup(arg);
	return;
}

int
can_speak_language(Creature * ch, char language_id)
{
	if (GET_LEVEL(ch) >= LVL_AMBASSADOR)
		return 1;
    
    if (language_id < 0)
        return 1;

	return ((KNOWN_LANGUAGES(ch) & ((long long)1 << language_id)) != 0);
}

ACMD(do_speak_language)
{
	char *language;
	int language_idx;

	if (!*argument) {
		send_to_char(ch, "You are currently speaking %s.\r\n",
			((GET_LANGUAGE(ch) > LANGUAGE_COMMON) ?
				language_names[(int)GET_LANGUAGE(ch)] : "common"));
		return;
	}

	language = tmp_getword(&argument);
	if (isname(language, "common")) {
		GET_LANGUAGE(ch) = LANGUAGE_COMMON;
		send_to_char(ch, "Ok, you're now speaking common.\r\n");
		return;
	}

	language_idx = search_block(language, language_names, FALSE);

	if (language_idx < 0) {
		send_to_char(ch, "That's not a language!\r\n");
		return;
	}

	if (can_speak_language(ch, language_idx)) {
		GET_LANGUAGE(ch) = language_idx;
		send_to_char(ch, "Ok, you're now speaking %s.\r\n",
			language_names[language_idx]);
	} else
		send_to_char(ch, "You don't know that language!\r\n");
}

ACMD(do_show_language)
{
	int idx = GET_LANGUAGE(ch);
	int num_languages = 1;

	send_to_char(ch, "%sYou are currently speaking:  %s%s\r\n\r\n",
		CCCYN(ch, C_NRM), CCNRM(ch, C_NRM),
		((GET_LANGUAGE(ch) > LANGUAGE_COMMON) ?
			tmp_capitalize(language_names[idx]) : "Common"));
	send_to_char(ch, "%sYou are fluent in the following languages:%s\r\n",
		CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
	send_to_char(ch, "%-15s", "Common");
	for (int x = 0; x < NUM_LANGUAGES; x++) {
		if (can_speak_language(ch, x)) {
			num_languages++;
			send_to_char(ch, "%-15s", tmp_capitalize(language_names[x]));
			if (num_languages % 4 == 0)
				send_to_char(ch, "\r\n");
		}
	}

	send_to_char(ch, "\r\n");
}

void
set_initial_language(Creature * ch)
{
	int language_idx;

	language_idx = find_language_idx_by_race(player_race[(int)GET_RACE(ch)]);

	if (language_idx != LANGUAGE_NONE && known_languages(ch) == 0) {
		learn_language(ch, language_idx);
	}

	if ((language_idx == LANGUAGE_NONE) ||
		!can_speak_language(ch, GET_LANGUAGE(ch)))
		GET_LANGUAGE(ch) = LANGUAGE_COMMON;
}

void
learn_language(Creature * ch, char language_idx)
{
	KNOWN_LANGUAGES(ch) |= ((long long)1 << language_idx);
}

void
forget_language(Creature * ch, char language_idx)
{
	KNOWN_LANGUAGES(ch) &= ~((long long)1 << language_idx);
}

int
known_languages(Creature * ch)
{
	int counter = 0;

	for (int x = 0; x < NUM_LANGUAGES; x++) {
		if (can_speak_language(ch, x))
			counter++;
	}

	return counter;
}
