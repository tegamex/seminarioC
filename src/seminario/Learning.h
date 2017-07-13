#ifndef LEARNING_H_
#define LEARNING_H_

#include "../defines.h"

namespace seminario
{
class Learning
{
public:
	explicit Learning();
	virtual ~Learning();
	static SUnique< Learning >& getInstance();
	void train( const Value& params , SString& output );
	void predict( const Value& params , SString& output );
private:
	bool getFileAndExist( const Value& params , SString& filename );
	bool getFileAndExist( const Value& params , SString& filenameTrained , SString& filename );
	const static SString filesInputDirectory;
	const static SString filesOutputDirectory;
	static SUnique< Learning > m_instance;
};
inline SUnique< Learning >& Learning::getInstance()
{
	if( !m_instance )
	{
		m_instance = std::make_unique< Learning >();
	}
	return m_instance;
}
}
#endif