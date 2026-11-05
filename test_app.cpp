#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <chrono>
using namespace map_project;

TEST_CASE("health endpoint payload includes status and uptime") {
    const auto payload = HealthBody(std::chrono::steady_clock::now());
    CHECK(payload.at("status") == "ok");
    CHECK(payload.at("uptime_seconds").get<long>() >= 0);
}

TEST_CASE("version payload exposes build metadata") {
    const auto payload = VersionBody();
    CHECK(payload.at("app") == kAppName);
    CHECK(payload.at("version") == kAppVersion);
    CHECK(payload.contains("commit"));
    CHECK(payload.contains("built_at"));
}

TEST_CASE("contact can be inserted and retrieved") {
    Store agenda;
    const auto added = agenda.AddContact("Ion Popescu", "ion@example.ro", "0712345678", "friends");
    REQUIRE(added.has_value());
    CHECK(added->id == 1);
    const auto found = agenda.GetContactById(added->id);
    REQUIRE(found.has_value());
    CHECK(found->name == "Ion Popescu");
}

TEST_CASE("email uniqueness ignores letter case") {
    Store agenda;
    REQUIRE(agenda.AddContact("Ana", "ana@example.ro", "0711111111", "family"));
    CHECK_FALSE(agenda.AddContact("Alt nume", "ANA@example.ro", "0722222222", "work"));
    CHECK(agenda.EmailExists("Ana@Example.ro"));
}

TEST_CASE("contacts can be filtered by category and query") {
    Store agenda;
    REQUIRE(agenda.AddContact("Mihai Ionescu", "mihai@example.ro", "0711111111", "work"));
    REQUIRE(agenda.AddContact("Ana Pop", "ana@example.ro", "0722222222", "family"));
    REQUIRE(agenda.AddContact("Mihai Georgescu", "mg@example.ro", "0733333333", "friends"));

    const auto filtered = agenda.GetContacts(std::string("work"), std::string("MIHAI"));
    REQUIRE(filtered.size() == 1);
    CHECK(filtered.front().name == "Mihai Ionescu");
}

TEST_CASE("search can match email or phone") {
    Store agenda;
    REQUIRE(agenda.AddContact("Ana", "ana.pop@example.ro", "0712345678", "family"));
    CHECK(agenda.GetContacts(std::nullopt, std::string("pop@")).size() == 1);
    CHECK(agenda.GetContacts(std::nullopt, std::string("123456")).size() == 1);
}

TEST_CASE("contacts are sorted without case sensitivity") {
    Store agenda;
    REQUIRE(agenda.AddContact("vasile", "v@example.ro", "0711111111", "friends"));
    REQUIRE(agenda.AddContact("Ana", "a@example.ro", "0722222222", "family"));
    const auto contacts = agenda.GetContacts();
    REQUIRE(contacts.size() == 2);
    CHECK(contacts[0].name == "Ana");
    CHECK(contacts[1].name == "vasile");
}

TEST_CASE("contact can be updated and deleted") {
    Store agenda;
    const auto created = agenda.AddContact("Ion", "ion@example.ro", "0712345678", "work");
    REQUIRE(created);
    const auto updated = agenda.UpdateContact(created->id, "Ion Popescu",
        "ion.p@example.ro", "0798765432", "friends");
    REQUIRE(updated);
    CHECK(updated->name == "Ion Popescu");
    CHECK(agenda.DeleteContact(created->id));
    CHECK_FALSE(agenda.GetContactById(created->id));
    CHECK_FALSE(agenda.DeleteContact(created->id));
}

TEST_CASE("updating a contact rejects another contact's email") {
    Store agenda;
    const auto first = agenda.AddContact("Ana", "ana@example.ro", "0711111111", "family");
    const auto second = agenda.AddContact("Mihai", "mihai@example.ro", "0722222222", "work");
    REQUIRE(first);
    REQUIRE(second);
    CHECK_FALSE(agenda.UpdateContact(second->id, "Mihai", "ana@example.ro",
        "0722222222", "work"));
}

TEST_CASE("reset clears records and restarts identifier sequence") {
    Store agenda;
    REQUIRE(agenda.AddContact("Ana", "ana@example.ro", "0711111111", "family"));
    agenda.Reset();
    CHECK(agenda.GetContacts().empty());
    CHECK(agenda.NextId() == 1);
}

TEST_CASE("input validators detect invalid values") {
    CHECK(IsValidEmail("alex@example.ro"));
    CHECK_FALSE(IsValidEmail("alex.example.ro"));
    CHECK(IsValidPhone("0712345678"));
    CHECK_FALSE(IsValidPhone("07123abc"));
    CHECK(Trim("  Ana  ") == "Ana");
}

TEST_CASE("statistics count contacts and categories") {
    Store agenda;
    REQUIRE(agenda.AddContact("Ana", "ana@example.ro", "0711111111", "family"));
    REQUIRE(agenda.AddContact("Mihai", "mihai@example.ro", "0722222222", "family"));
    REQUIRE(agenda.AddContact("Dan", "dan@example.ro", "0733333333", "work"));
    const auto stats = agenda.GetStats();
    CHECK(stats.at("total") == 3);
    CHECK(stats.at("by_category").at("family") == 2);
    CHECK(stats.at("by_category").at("work") == 1);
}

TEST_CASE("home page identifies the application author") {
    const auto page = HomePage();
    CHECK(page.find(kAppName) != std::string::npos);
    CHECK(page.find("Lăzărescu Alexandru-Marcel") != std::string::npos);
    CHECK(page.find("grupa 1.2") != std::string::npos);
}
