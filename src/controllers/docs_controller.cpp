#include "controllers/docs_controller.h"
#include <fstream>
#include <sstream>

DocsController::DocsController(httplib::Server &srv) {
  // serve openapi.json
  srv.Get("/openapi.json", [](const httplib::Request &, httplib::Response &res){
    std::ifstream f("docs/openapi.json");
    if (!f) { res.status = 500; res.set_content("{}", "application/json"); return; }
    std::stringstream ss; ss << f.rdbuf();
    res.set_content(ss.str(), "application/json");
  });

  // serve a minimal swagger UI page using public CDN and the local /openapi.json
  srv.Get("/docs", [](const httplib::Request &, httplib::Response &res){
    static const char html[] = R"(<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <title>Swagger UI</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.18.2/swagger-ui.css" />
  </head>
  <body>
    <div id="swagger-ui"></div>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.18.2/swagger-ui-bundle.js"></script>
    <script>
      window.onload = function() {
        const ui = SwaggerUIBundle({
          url: '/openapi.json',
          dom_id: '#swagger-ui'
        });
      };
    </script>
  </body>
</html>)";
    res.set_content(html, "text/html; charset=utf-8");
  });
}
