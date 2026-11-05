#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace map_project {

inline constexpr const char* kAppName = "agenda-contacte";
inline constexpr const char* kAppVersion = "1.0.0";

struct Contact {
    int id{};
    std::string name;
    std::string email;
    std::string phone;
    std::string category;

    nlohmann::json ToJson() const {
        return {{"id", id}, {"name", name}, {"email", email},
                {"phone", phone}, {"category", category}};
    }
};

inline std::string Trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(),
        [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(value.rbegin(), value.rend(),
        [](unsigned char ch) { return std::isspace(ch); }).base();
    if (first >= last) return {};
    return std::string(first, last);
}

inline std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

inline bool IsValidEmail(const std::string& email) {
    const auto at = email.find('@');
    if (at == std::string::npos || at == 0 || at != email.rfind('@')) return false;
    const auto dot = email.find('.', at + 2);
    return dot != std::string::npos && dot + 1 < email.size();
}

inline bool IsValidPhone(const std::string& phone) {
    return phone.size() == 10 &&
        std::all_of(phone.begin(), phone.end(),
            [](unsigned char ch) { return std::isdigit(ch); });
}

inline std::string Env(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return (value == nullptr || *value == '\0') ? fallback : std::string(value);
}

class Store {
public:
    void Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        contacts_.clear();
        next_id_ = 1;
    }

    int NextId() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return next_id_;
    }

    bool EmailExists(const std::string& email) const {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto wanted = ToLower(Trim(email));
        return std::any_of(contacts_.begin(), contacts_.end(),
            [&](const Contact& contact) { return ToLower(contact.email) == wanted; });
    }

    std::optional<Contact> AddContact(const std::string& name,
        const std::string& email, const std::string& phone,
        const std::string& category) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto normalized_email = ToLower(Trim(email));
        if (std::any_of(contacts_.begin(), contacts_.end(),
            [&](const Contact& item) { return ToLower(item.email) == normalized_email; })) {
            return std::nullopt;
        }
        Contact contact{next_id_++, Trim(name), Trim(email), Trim(phone), Trim(category)};
        contacts_.push_back(contact);
        return contact;
    }

    std::optional<Contact> GetContactById(int id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& contact : contacts_) {
            if (contact.id == id) return contact;
        }
        return std::nullopt;
    }

    bool DeleteContact(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto old_size = contacts_.size();
        contacts_.erase(std::remove_if(contacts_.begin(), contacts_.end(),
            [id](const Contact& item) { return item.id == id; }), contacts_.end());
        return contacts_.size() != old_size;
    }

    std::optional<Contact> UpdateContact(int id, const std::string& name,
        const std::string& email, const std::string& phone,
        const std::string& category) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto normalized_email = ToLower(Trim(email));
        for (const auto& item : contacts_) {
            if (item.id != id && ToLower(item.email) == normalized_email) return std::nullopt;
        }
        for (auto& item : contacts_) {
            if (item.id == id) {
                item.name = Trim(name);
                item.email = Trim(email);
                item.phone = Trim(phone);
                item.category = Trim(category);
                return item;
            }
        }
        return std::nullopt;
    }

    std::vector<Contact> GetContacts(
        const std::optional<std::string>& category = std::nullopt,
        const std::optional<std::string>& query = std::nullopt) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Contact> result;
        const auto wanted_category = category ? ToLower(Trim(*category)) : "";
        const auto wanted_query = query ? ToLower(Trim(*query)) : "";
        for (const auto& item : contacts_) {
            if (!wanted_category.empty() && ToLower(item.category) != wanted_category) continue;
            if (!wanted_query.empty()) {
                const auto matches = [&](const std::string& field) {
                    return ToLower(field).find(wanted_query) != std::string::npos;
                };
                if (!matches(item.name) && !matches(item.email) && !matches(item.phone)) continue;
            }
            result.push_back(item);
        }
        std::sort(result.begin(), result.end(), [](const Contact& a, const Contact& b) {
            const auto left = ToLower(a.name);
            const auto right = ToLower(b.name);
            return left == right ? a.id < b.id : left < right;
        });
        return result;
    }

    nlohmann::json GetStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::string, int> categories;
        for (const auto& item : contacts_) ++categories[item.category];
        return {{"total", contacts_.size()}, {"by_category", categories}};
    }

private:
    mutable std::mutex mutex_;
    int next_id_ = 1;
    std::vector<Contact> contacts_;
};

inline long UptimeSeconds(std::chrono::steady_clock::time_point started_at) {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - started_at).count();
}

inline nlohmann::json HealthBody(std::chrono::steady_clock::time_point started_at) {
    return {{"status", "ok"}, {"uptime_seconds", UptimeSeconds(started_at)}};
}

inline nlohmann::json VersionBody() {
    return {{"app", kAppName}, {"version", kAppVersion},
            {"commit", Env("APP_COMMIT", "dev")},
            {"built_at", Env("APP_BUILT_AT", "unknown")}};
}

inline nlohmann::json ErrorBody(const std::string& code, const std::string& message) {
    return {{"error", code}, {"message", message}};
}

inline std::string HomePage() {
    return std::string(R"(<!doctype html>
<html lang="ro"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>)") + kAppName + R"(</title></head><body>
<main><h1>Agenda de contacte</h1>
<p>Autor: Lăzărescu Alexandru-Marcel | Grupa 1.2 | Tema 1</p>
<p>Serviciu REST pentru administrarea contactelor.</p>
<p>Versiune: )" + kAppVersion + R"(</p></main></body></html>)";
}

} // namespace map_project
