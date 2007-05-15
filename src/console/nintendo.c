/*
nintendo.c - _publisher_ strings for SNES, GB, GBC and GBA games

Copyright (c) 2003 - 2007 NoisyB


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include "nintendo.h"


const char *nintendo_maker[NINTENDO_MAKER_LEN] =
  {
    NULL, "Nintendo", "Rocket Games/Ajinomoto", "Imagineer-Zoom", "Gray Matter",
    "Zamuse", "Falcom", NULL, "Capcom", "Hot B Co.",
    "Jaleco", "Coconuts Japan", "Coconuts Japan/G.X.Media",
      "Micronet", "Technos",
    "Mebio Software", "Shouei System", "Starfish", NULL, "Mitsui Fudosan/Dentsu",
    NULL, "Warashi Inc.", NULL, "Nowpro", NULL,
    "Game Village", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 0Z
    NULL, "Starfish", "Infocom", "Electronic Arts Japan", NULL,
    "Cobra Team", "Human/Field", "KOEI", "Hudson Soft", "S.C.P./Game Village",
    "Yanoman", NULL, "Tecmo Products", "Japan Glary Business", "Forum/OpenSystem",
    "Virgin Games (Japan)", "SMDE", NULL, NULL, "Daikokudenki",
    NULL, NULL, NULL, NULL, NULL,
    "Creatures Inc.", "TDK Deep Impresion", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 1Z
    "Destination Software/KSS", "Sunsoft/Tokai Engineering",
      "POW (Planning Office Wada)/VR 1 Japan", "Micro World", NULL,
    "San-X", "Enix", "Loriciel/Electro Brain", "Kemco Japan", "Seta Co., Ltd.",
    "Culture Brain", NULL, "Palsoft", "Visit Co., Ltd.", "Intec",
    "System Sacom", "Poppo", "Ubisoft Japan", NULL, "Media Works",
    "NEC InterChannel", "Tam", "Gajin/Jordan", "Smilesoft", NULL,
    NULL, "Mediakite", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 2Z
    "Viacom", "Carrozzeria", "Dynamic", NULL, "Magifact",
    "Hect", "Codemasters", "Taito/GAGA Communications", "Laguna",
      "Telstar Fun & Games/Event/Taito",
    NULL, "Arcade Zone Ltd.", "Entertainment International/Empire Software", "Loriciel",
      "Gremlin Graphics",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 3Z
    "Seika Corp.", "UBI SOFT Entertainment Software", "Sunsoft US", NULL, "Life Fitness",
    NULL, "System 3", "Spectrum Holobyte", NULL, "IREM",
    NULL, "Raya Systems", "Renovation Products", "Malibu Games", NULL,
    "Eidos/U.S. Gold", "Playmates Interactive", NULL, NULL, "Fox Interactive",
    "Time Warner Interactive", NULL, NULL, NULL, NULL,
    NULL, "Disney Interactive", NULL, "Black Pearl", NULL,
    "Advanced Productions", NULL, NULL, "GT Interactive", "RARE",
    "Crave Entertainment",                      // 4Z
    "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "Take 2/GameTek",
    "Hi Tech", "LJN Ltd.", NULL, "Mattel", NULL,
    "Mindscape/Red Orb Entertainment", "Romstar", "Taxan", "Midway/Tradewest", NULL,
    "American Softworks Corp.", "Majesco Sales Inc.", "3DO", NULL, NULL,
    "Hasbro", "NewKidCo", "Telegames", "Metro3D", NULL,
    "Vatical Entertainment", "LEGO Media", NULL, "Xicat Interactive", "Cryo Interactive",
    NULL, NULL, "Red Storm Entertainment", "Microids", NULL,
    "Conspiracy/Swing",                         // 5Z
    "Titus", "Virgin Interactive", "Maxis", NULL, "LucasArts Entertainment",
    NULL, NULL, "Ocean", NULL, "Electronic Arts",
    NULL, "Laser Beam", NULL, NULL, "Elite Systems",
    "Electro Brain", "The Learning Company", "BBC", NULL, "Software 2000",
    NULL, "BAM! Entertainment", "Studio 3", NULL, NULL,
    NULL, "Classified Games", NULL, "TDK Mediactive", NULL,
    "DreamCatcher", "JoWood Produtions", "SEGA", "Wannado Edition",
      "LSP (Light & Shadow Prod.)",
    "ITE Media",                                // 6Z
    "Infogrames", "Interplay", "JVC (US)", "Parker Brothers", NULL,
    "SCI (Sales Curve Interactive)/Storm", NULL, NULL, "THQ Software", "Accolade Inc.",
    "Triffix Entertainment", NULL, "Microprose Software",
      "Universal Interactive/Sierra/Simon & Schuster", NULL,
    "Kemco", "Rage Software", "Encore", NULL, "Zoo",
    "BVM", "Simon & Schuster Interactive", "Asmik Ace Entertainment Inc./AIA",
      "Empire Interactive", NULL,
    NULL, "Jester Interactive", NULL, NULL, "Scholastic",
    "Ignition Entertainment", NULL, "Stadlbauer", NULL, NULL,
    NULL,                                       // 7Z
    "Misawa", "Teichiku", "Namco Ltd.", "LOZC", "KOEI",
    NULL, "Tokuma Shoten Intermedia", "Tsukuda Original", "DATAM-Polystar", NULL,
    NULL, "Bulletproof Software", "Vic Tokai Inc.", NULL, "Character Soft",
    "I'Max", "Saurus", NULL, NULL, "General Entertainment",
    NULL, NULL, "I'Max", "Success", NULL,
    "SEGA Japan", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 8Z
    "Takara", "Chun Soft", "Video System Co., Ltd./McO'River", "BEC", NULL,
    "Varie", "Yonezawa/S'pal", "Kaneko", NULL, "Victor Interactive Software/Pack in Video",
    "Nichibutsu/Nihon Bussan", "Tecmo", "Imagineer", NULL, NULL,
    "Nova", "Den'Z", "Bottom Up", NULL, "TGL (Technical Group Laboratory)",
    NULL, "Hasbro Japan", NULL, "Marvelous Entertainment", NULL,
    "Keynet Inc.", "Hands-On Entertainment", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 9Z
    "Telenet", "Hori", NULL, NULL, "Konami",
    "K.Amusement Leasing Co.", "Kawada", "Takara", NULL, "Technos Japan Corp.",
    "JVC (Europe/Japan)/Victor Musical Industries", NULL, "Toei Animation", "Toho", NULL,
    "Namco", "Media Rings Corp.", "J-Wing", NULL, "Pioneer LDC",
    "KID", "Mediafactory", NULL, NULL, NULL,
    "Infogrames Hudson", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // AZ
    "Acclaim Japan", "ASCII Co./Nexoft" /* Activision */, "Bandai", NULL, "Enix",
    NULL, "HAL Laboratory/Halken", "SNK", NULL, "Pony Canyon Hanbai",
    "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony Imagesoft", NULL,
    "Sammy", "Magical", "Visco", NULL, "Compile",
    NULL, "MTO Inc.", NULL, "Sunrise Interactive", NULL,
    "Global A Entertainment", "Fuuki", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // BZ
    "Taito", NULL, "Kemco", "Square", "Tokuma Shoten",
    "Data East", "Tonkin House", NULL, "KOEI", NULL,
    "Konami/Ultra/Palcom", "NTVIC/VAP", "Use Co., Ltd.", "Meldac",
      "Pony Canyon (Japan)/FCI (US)",
    "Angel/Sotsu Agency/Sunrise", "Yumedia/Aroma Co., Ltd.", NULL, NULL, "Boss",
    "Axela/Crea-Tech", "Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
      "Konami Computer Entertainment Osaka", NULL, NULL,
    "Enterbrain", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // CZ
    "Taito/Disco", "Sofel", "Quest Corp.", "Sigma", "Ask Kodansha",
    NULL, "Naxat", "Copya System", "Capcom Co., Ltd.", "Banpresto",
    "TOMY", "Acclaim/LJN Japan", NULL, "NCS", "Human Entertainment",
    "Altron", "Jaleco", "Gaps Inc.", NULL, NULL,
    NULL, NULL, NULL, "Elf", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // DZ
    "Jaleco", NULL, "Yutaka", "Varie", "T&ESoft",
    "Epoch Co., Ltd.", NULL, "Athena", "Asmik", "Natsume",
    "King Records", "Atlus", "Epic/Sony Records (Japan)", NULL,
      "IGS (Information Global Service)",
    NULL, "Chatnoir", "Right Stuff", NULL, NULL,
    NULL, "Spike", "Konami Computer Entertainment Tokyo", "Alphadream Corp.", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // EZ
    "A Wave", "Motown Software", "Left Field Entertainment", "Extreme Ent. Grp.",
      "TecMagik",
    NULL, NULL, NULL, NULL, "Cybersoft",
    NULL, "Psygnosis", NULL, NULL, "Davidson/Western Tech.",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // FZ
    NULL, "PCCW Japan", NULL, NULL, "KiKi Co. Ltd.",
    "Open Sesame Inc.", "Sims", "Broccoli", "Avex", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // GZ
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // HZ
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, "Yojigen", NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL                                        // IZ
  };
