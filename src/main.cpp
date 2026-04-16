#include "app/Application.hpp"
#include "core/AppConfig.hpp"

int main() {
    traffic::core::AppConfig config;
    traffic::app::Application app(config);
    app.run();
    return 0;
}

