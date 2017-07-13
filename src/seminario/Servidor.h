#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "../defines.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
typedef websocketpp::server<websocketpp::config::asio> websocket_server;

namespace seminario
{
class Servidor
{
public:
	explicit Servidor();
	virtual ~Servidor();
	static SUnique< Servidor >& getInstance();
	void startServer();
	void websocketOnOpen( websocketpp::connection_hdl& hdl );
	void websocketOnHttp( websocketpp::connection_hdl& hdl );
	void websocketOnMessage( websocketpp::connection_hdl& hdl , websocket_server::message_ptr& msg );
	void uploadToTrain( const Value& params , SString& output );
	void uploadToPredict( const Value& params , SString& output );
	void otherMessages( websocketpp::connection_hdl& hdl , Document& document );
protected:
private:
	static SUnique< Servidor > m_instance;
	SUnique< websocket_server > m_wsServer;
};
inline SUnique< Servidor >& Servidor::getInstance()
{
	if( !m_instance )
	{
		m_instance = std::make_unique< Servidor >();
	}
	return m_instance;
}
}
#endif