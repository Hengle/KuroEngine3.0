#include "Object.h"
#include"Importer.h"

ModelObject::ModelObject(const std::string& Dir, const std::string& FileName)
{
	model = Importer::Instance()->LoadModel(Dir, FileName);
}
