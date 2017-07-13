#include "Servidor.h"
#include "Learning.h"

using seminario::Servidor;
using seminario::Learning;

SUnique< Servidor > Servidor::m_instance= nullptr;

Servidor::Servidor()
{}

Servidor::~Servidor()
{}

void websocket_on_http(websocketpp::connection_hdl hdl) 
{
    Servidor::getInstance()->websocketOnHttp( hdl );
}

void websocket_on_open(websocketpp::connection_hdl hdl)
{
    Servidor::getInstance()->websocketOnOpen( hdl );
}

void websocket_on_message( websocketpp::connection_hdl hdl, websocket_server::message_ptr msg )
{
    Servidor::getInstance()->websocketOnMessage( hdl , msg );
}

void Servidor::startServer()
{
    m_wsServer = std::make_unique< websocket_server >();
    m_wsServer->init_asio();
    m_wsServer->set_http_handler(&websocket_on_http);
    m_wsServer->set_open_handler(&websocket_on_open);
    m_wsServer->set_message_handler(&websocket_on_message);
    m_wsServer->set_reuse_addr(true);
    m_wsServer->listen(9002);
    m_wsServer->start_accept();
    m_wsServer->run();
}

void Servidor::websocketOnHttp( websocketpp::connection_hdl& hdl ) 
{
    websocket_server::connection_ptr con = m_wsServer->get_con_from_hdl(hdl);
    con->set_status( websocketpp::http::status_code::ok );
    con->append_header( "access-control-allow-origin" , "*" );
    con->append_header( "content-type", "application/json; charset=UTF-8" );
    string message="{ websocket: true, hasApi: false }";
    con->set_body( message.c_str() );
}

void Servidor::websocketOnOpen( websocketpp::connection_hdl& hdl ) 
{
    m_wsServer->send( hdl , "o", 1 , websocketpp::frame::opcode::text );
}

void Servidor::websocketOnMessage( websocketpp::connection_hdl& hdl , websocket_server::message_ptr& msg )
{
    SString message = msg->get_payload();
    cout<<"mensaje obtenido: "<<message<<endl;
    char buffer[message.size() + 1 ];
    memcpy( buffer , message.c_str() , message.size() );
    buffer[ message.size() ]='\0';
    Document document;
    if (document.ParseInsitu(buffer).HasParseError())
    {
        cout << "Error in Parsing" << endl;
        return;
    }
    if( !document.HasMember("msg") )
    {
        cout << "There is no msg field" << endl;
        return;
    }
    if( !document["msg"].IsString() )
    {
        cout << "msg field is no string" << endl;
        return;
    }
    if( strcmp( document["msg"].GetString() , "method" ) !=0 )
    {
        otherMessages( hdl , document );
        return;
    }
    /////////////////////////
    if( !document.HasMember("method") )
    {
        cout << "There is no method field" << endl;
        return;
    }
    if( !document["method"].IsString() )
    {
        cout << "method field is no string" << endl;
        return;
    }
    int state = 0;
    if( strcmp( document["method"].GetString() , "train" ) == 0 )
    {
        state = 1;
    }
    else if( strcmp( document["method"].GetString() , "predict" ) == 0 )
    {
        state = 2;
    }
    if( !document.HasMember("params") )
    {
        cout << "There is no params field" << endl;
        return;
    }
    ////////////////////
    SString output;
    switch( state )
    {
        case 1:
        {
            uploadToTrain( document["params"] , output );
            break;
        }
        case 2:
        {
            uploadToPredict( document["params"] , output );
            break;
        }
        default:
        {
            cout << "error name function" << endl;
            return;
        }
    }

    StringBuffer s;
    Writer< StringBuffer > writer( s );
    //writer.StartObject();
    //writer.StartArray(); 
    
    writer.StartObject();
    writer.Key("msg");           
    writer.String("result");  
    writer.Key("id");           
    writer.String(document["id"].GetString());  
    writer.Key("result");     
    cout << 1 ;      
    writer.String( output.c_str() );  
    cout << 2 ;
    writer.EndObject();
    /*
    writer.StartObject();
    writer.Key("msg");       
    cout <<3 ;
    writer.String("updated");  
    writer.Key("methods"); 
    writer.StartArray(); 
    cout << 4;
    writer.String(document["id"].GetString());  
    writer.EndArray();
    writer.EndObject();
    cout << 5;
    writer.EndArray();
    //writer.EndObject();
    */
    SString salida;
    salida.append( s.GetString() );
    m_wsServer->send( hdl , salida.c_str() , salida.size()  , websocketpp::frame::opcode::text );
}

void Servidor::uploadToTrain( const Value& params , SString& output )
{
    Learning::getInstance()->train( params , output );
}

void Servidor::uploadToPredict( const Value& params , SString& output )
{
    Learning::getInstance()->predict( params , output );
}

SString getRandomId(int len)
{
    SString s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return s;
}

void Servidor::otherMessages( websocketpp::connection_hdl& hdl , Document& document )
{
    StringBuffer s;
    Writer< StringBuffer > writer( s );
    writer.StartObject();  
    if( strcmp( document["msg"].GetString() , "ping" ) == 0 )
    {
        writer.Key("msg");           
        writer.String("pong");
        if( document.HasMember("id") )
        {
            writer.Key("id");           
            writer.String( document["id"].GetString() );
        }
    }
    else if( strcmp( document["msg"].GetString() , "connect" ) == 0 )
    {
        writer.Key("msg");           
        writer.String("connected");
        if( document.HasMember("session") )
        {
            writer.Key("session");           
            writer.String( document["session"].GetString() );
        }
        else
        {
            writer.Key("session");           
            writer.String( getRandomId(10).c_str() );
        }
    }
    else if( strcmp( document["msg"].GetString() , "sub" ) == 0 )
    {
        writer.StartArray(); 
        writer.String(document["id"].GetString()); 
        writer.EndArray();
        writer.Key("msg");           
        writer.String("ready");
        writer.Key("subs");           
        writer.String("subs");
    }
    else if( strcmp( document["msg"].GetString() , "unsub" ) == 0 )
    {}
    writer.EndObject();
    SString output = s.GetString();
    m_wsServer->send( hdl , output.c_str() , output.size()  , websocketpp::frame::opcode::text );
}