#include <chrono>
#include <iostream>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "app.hpp"

using namespace map_project;
using nlohmann::json;

int main() {
    const auto started_at = std::chrono::steady_clock::now();
    Store agenda;
    httplib::Server server;

    server.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(HealthBody(started_at).dump(), "application/json; charset=utf-8");
    });

    server.Get("/version", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(VersionBody().dump(), "application/json; charset=utf-8");
    });

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(HomePage(), "text/html; charset=utf-8");
    });

    // Primele rute pentru consultarea si adaugarea contactelor.
    server.Get("/contacts", [&](const httplib::Request&, httplib::Response& res) {
        json items = json::array();
        for (const auto& contact : agenda.GetContacts()) {
            items.push_back(contact.ToJson());
        }
        res.set_content(json{{"contacts", items}, {"count", items.size()}}.dump(),
                        "application/json; charset=utf-8");
    });

    server.Post("/contacts", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto body = json::parse(req.body);
            if (!body.contains("name") || !body.contains("email") ||
                !body.contains("phone") || !body.contains("category")) {
                res.status = 400;
                res.set_content(ErrorBody("bad_request", "Lipsesc campuri obligatorii.").dump(),
                                "application/json; charset=utf-8");
                return;
            }

            const auto added = agenda.AddContact(
                body.at("name").get<std::string>(),
                body.at("email").get<std::string>(),
                body.at("phone").get<std::string>(),
                body.at("category").get<std::string>());

            if (!added) {
                res.status = 409;
                res.set_content(ErrorBody("duplicate_email", "Email deja existent.").dump(),
                                "application/json; charset=utf-8");
                return;
            }

            res.status = 201;
            res.set_content(added->ToJson().dump(), "application/json; charset=utf-8");
        } catch (const json::exception&) {
            res.status = 400;
            res.set_content(ErrorBody("invalid_json", "Cerere JSON invalida.").dump(),
                            "application/json; charset=utf-8");
        }
    });

    // TODO: urmeaza rutele pentru detalii, modificare si stergere.
    std::cout << kAppName << " - versiune intermediara, port 8080\n";
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "Nu s-a putut porni serverul pe portul 8080.\n";
        return 1;
    }
    return 0;
}
