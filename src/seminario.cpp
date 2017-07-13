#include "seminario/Servidor.h"

using seminario::Servidor;

int main(int argc, char** argv)
{
	auto& ddpServer = Servidor::getInstance();
	ddpServer->startServer();
}
