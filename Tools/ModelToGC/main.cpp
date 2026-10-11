using namespace std;
#include <string.h>
#include <iostream>

// What IS an obj file?
// Note: This is taken from StaticMesh.h in GameCube Engine
struct StaticMesh
{
	char TypeIdentifier[4]{ 'G','C','S','M' };
	unsigned int VersionNumber = 1;

	// Counts
	unsigned int IndexCount;
	unsigned int VertexCount;

	// Offsets
	unsigned int IndexArrayOffset;
	unsigned int VertexArrayOffset;
	unsigned int UVOffset;
	unsigned int NormalOffset;
	unsigned int ColorOffset;

	// Because the cache lines of the Gamecube are 32 bytes and it expects exact values, we just throw some padding
	// into this object to ensure its going to be 2 cache lines long!
	// The logic is either 32 or 64 bytes. If we have 36 (as we currently do without padding) then we need to pad out
	// those extra bytes so it matches those exact 2 lines.
	char padding[28];
};

int main(int argc, char* argv[])
{

	return 0;
}


void ProcessArguments(int args, char* cmds[])
{
	for (int i = 0; i < args; i++)
	{

	}
}

/*
* When we process the file we assume its going to follow the format spat out by a Blender file
* Eg...
* v -100.00000 -100.00000 100.00000
* vn -1.0000 -0.0000 -0.0000
* vt 0.000087 0.749957
* s 0
* f 1/1/1 2/2/1 4/3/1 3/4/1
*
* So we're using the Spaces as the parser.
*/
void ProcessObjFile(const char* objFile)
{
	StaticMesh Mesh;

	// Open File
	const char* line;
	int lineNum = 0;
	while (true)
	{
		if (line[0] == '#')
		{
			// Next line
			continue;
		}

		if (line[1] != ' ')
		{
			std::cout << "Error: Malformed line " << lineNum << '\n';
			break;
		}

		if (line[0] == 'v')
		{
			char subLine;
			while(subLine != '\0')
		}

		if (line[0] == 's')
		{
		}

		if (line[0] == 'f')
		{
		}

		if (strncmp(line, "vn", 2))
		{
		}

		if (strncmp(line, "vt", 2))
		{
		}
	};
};

void PassData()
{

}