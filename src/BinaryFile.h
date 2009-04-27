// $Id: BinaryFile.h 4652 2009-03-29 10:10:02Z FloSoft $
//
// Copyright (c) 2005-2009 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Siedler II.5 RTTR.
//
// Siedler II.5 RTTR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Siedler II.5 RTTR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Siedler II.5 RTTR. If not, see <http://www.gnu.org/licenses/>.
#ifndef BINARY_FILE_H_
#define BINARY_FILE_H_

#include <stdio.h>
#include <string>

class String;

// �ffnungsmethoden
enum OpenFileMode
{
	OFM_WRITE = 0, 
	OFM_READ
};

/// Klasse f�r das Laden und Speichern von bin�ren Dateien
class BinaryFile
{
private:

	/// File-Pointer
	FILE * file;

public:

	BinaryFile() : file(0) {}
	~BinaryFile() { Close(); }


	/// �ffnet eine Datei: liefert true falls erfolgreich
	bool Open(const char * const filename, const OpenFileMode of);
	/// Schlie�t eine Datei: liefert true falls erfolgreich
	bool Close();

	/// Schreibmethoden
	void WriteSignedInt(signed int i) { fwrite(&i,4,1,file); }
	void WriteUnsignedInt(unsigned int i) { fwrite(&i,4,1,file); }
	void WriteSignedShort(signed short i) { fwrite(&i,2,1,file); }
	void WriteUnsignedShort(unsigned short i) { fwrite(&i,2,1,file); }
	void WriteSignedChar(signed char i) { fwrite(&i,1,1,file); }
	void WriteUnsignedChar(unsigned char i) { fwrite(&i,1,1,file); }

	void WriteRawData(const void * const data, const unsigned length)
	{ fwrite(data,length,1,file); }

	void WriteShortString(const std::string& str); /// L�nge max 254
	void WriteLongString(const std::string& str); /// L�nge max 2^32-2

	/// Lesemethoden
	signed int ReadSignedInt() 
	{ signed int i; fread(&i,4,1,file); return i; }
	unsigned int ReadUnsignedInt()
	{ unsigned int i; fread(&i,4,1,file); return i; }
	signed short ReadSignedShort()
	{ signed short i; fread(&i,2,1,file); return i; }
	unsigned short ReadUnsignedShort() 
	{ unsigned short i; fread(&i,2,1,file); return i; }
	signed char ReadSignedChar()
	{ signed char i; fread(&i,1,1,file); return i; }
	unsigned char ReadUnsignedChar()
	{ unsigned char i; fread(&i,1,1,file); return i; }

	void ReadRawData(void * const data, const unsigned length)
	{ fread(data,length,1,file); }

	void ReadShortString(std::string& str); /// L�nge max 254
	void ReadLongString(std::string& str); /// L�nge max 2^32-2

	/// Setzt den Dateizeiger
	void Seek(const long pos,const int origin) { fseek(file,pos,origin); }
	/// Liefert Den Dateizeiger
	unsigned Tell() { return ftell(file); }

	/// Schreibt alles richtig in die Datei
	void Flush() { fflush(file); }

	/// Datei zu Ende?
	bool EndOfFile() const { return feof(file)?true:false; }

	/// Datei g�ltig?
	bool IsValid() const { return file?true:false; }

};


#endif // !BINARY_FILE_H_
