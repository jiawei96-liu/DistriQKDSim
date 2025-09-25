#include "Web/controller/DemandController.hpp"
#include "Web/controller/StaticController.hpp"
#include "Web/controller/LinkController.hpp"
#include "Web/controller/SimController.hpp"
#include "Web/AppComponent.hpp"
#include "Web/WorkerComponent.hpp"

#include "Alg/DistributedIDGenerator.h"
#include "oatpp/network/Server.hpp"

#include <iostream>

void runMaster() {

  /* Register Components in scope of run() method */
  AppComponent components;

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create MyController and add all of its endpoints to router */
  router->addController(std::make_shared<DemandController>());
  router->addController(std::make_shared<LinkController>());
  router->addController(std::make_shared<SimController>());
  router->addController(StaticController::createShared());

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  /* Print info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  /* Run server */
  server.run();
  
}

void runWorker() {

  /* Register Components in scope of run() method */
  WorkerComponent components;

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create MyController and add all of its endpoints to router */
  router->addController(std::make_shared<DemandController>());
  router->addController(std::make_shared<LinkController>());
  router->addController(std::make_shared<SimController>());

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  /* Print info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  /* Run server */
  server.run();
  
}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();
    std::srand(999);
    if(argc < 2) {
        OATPP_LOGE("Main", "Missing role argument (master/worker)");
        return 1;
    }

    std::string role = argv[1];

    if (role == "master") {
        DistributedIDGenerator::next_id(0);
        ConfigReader::setStr("role","master");
        runMaster();
    } else if (role == "worker") {
        DistributedIDGenerator::next_id(1);
        ConfigReader::setStr("role","worker");
        runWorker();
    } else {
        OATPP_LOGE("Main", "Unknown role: %s", role.c_str());
        return 1;
    }

  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}