#define ZPP_BITS_INLINE_MODE 0
#include "test.h"

namespace test_pb_protocol2
{

using namespace zpp::bits::literals;

struct example
{
    zpp::bits::vint32_t i; // field number == 1

    constexpr auto operator<=>(const example &) const = default;
};

static_assert(
    zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<example>{{150}}>() ==
    "089601"_decode_hex);

static_assert(zpp::bits::from_bytes<zpp::bits::protobuf{}, "089601"_decode_hex,
                                    zpp::bits::unsized_t<example>>()
                  .i == 150);

struct nested_example
{
    example nested; // field number == 1
};

static_assert(zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<nested_example>{
                  {.nested = example{150}}}>() == "0a03089601"_decode_hex);

static_assert(zpp::bits::from_bytes<zpp::bits::protobuf{}, "0a03089601"_decode_hex,
                                    zpp::bits::unsized_t<nested_example>>()
                  .nested.i == 150);

struct nested_reserved_example
{
    [[no_unique_address]] zpp::bits::pb_reserved _1{}; // field number == 1
    [[no_unique_address]] zpp::bits::pb_reserved _2{}; // field number == 2
    example nested{};                                  // field number == 3
};

static_assert(
    zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<nested_reserved_example>{
        {.nested = example{150}}}>() == "1a03089601"_decode_hex);

static_assert(
    zpp::bits::from_bytes<zpp::bits::protobuf{}, "1a03089601"_decode_hex,
                          zpp::bits::unsized_t<nested_reserved_example>>()
        .nested.i == 150);


struct nested_explicit_id_example
{
    zpp::bits::pb_field<example, 3> nested{};  // field number == 3
};

static_assert(sizeof(nested_explicit_id_example) == sizeof(example));

static_assert(
    zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<nested_explicit_id_example>{
        {.nested = example{150}}}>() == "1a03089601"_decode_hex);

static_assert(
    zpp::bits::from_bytes<zpp::bits::protobuf{}, "1a03089601"_decode_hex,
                          zpp::bits::unsized_t<nested_explicit_id_example>>()
        .nested.i == 150);

struct nested_map_id_example
{
    example nested{};  // field number == 3

    using pb_options =
        std::tuple<zpp::bits::pb_map<1, 3>>;
};

static_assert(sizeof(nested_map_id_example) == sizeof(example));

static_assert(
    zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<nested_map_id_example>{
        {.nested = example{150}}}>() == "1a03089601"_decode_hex);

static_assert(
    zpp::bits::from_bytes<zpp::bits::protobuf{}, "1a03089601"_decode_hex,
                          zpp::bits::unsized_t<nested_map_id_example>>()
        .nested.i == 150);

struct nested_map_id_with_explicit_pb_options
{
    example nested{};  // field number == 3
};

auto pb_options(nested_map_id_with_explicit_pb_options) -> std::tuple<zpp::bits::pb_map<1, 3>>;

static_assert(
    zpp::bits::to_bytes<zpp::bits::protobuf{}, zpp::bits::unsized_t<nested_map_id_with_explicit_pb_options>{
        {.nested = example{150}}}>() == "1a03089601"_decode_hex);

static_assert(
    zpp::bits::from_bytes<zpp::bits::protobuf{}, "1a03089601"_decode_hex,
                          zpp::bits::unsized_t<nested_map_id_with_explicit_pb_options>>()
        .nested.i == 150);


struct repeated_integers
{
    std::vector<zpp::bits::vsint32_t> integers;
};

TEST(test_pb_protocol2, test_repeated_integers)
{
    auto [data, in, out] = zpp::bits::data_in_out(zpp::bits::no_size{}, zpp::bits::protobuf{});
    out(repeated_integers{.integers = {1, 2, 3, 4, -1, -2, -3, -4}})
        .or_throw();

    repeated_integers r;
    in(r).or_throw();

    EXPECT_EQ(
        r.integers,
        (std::vector<zpp::bits::vsint32_t>{1, 2, 3, 4, -1, -2, -3, -4}));
}

struct repeated_examples
{
    std::vector<example> examples;
};

TEST(test_pb_protocol2, test_repeated_example)
{
    auto [data, in, out] = zpp::bits::data_in_out(zpp::bits::no_size{}, zpp::bits::protobuf{});
    out(repeated_examples{.examples = {{1}, {2}, {3}, {4}, {-1}, {-2}, {-3}, {-4}}})
        .or_throw();

    repeated_examples r;
    in(r).or_throw();

    EXPECT_EQ(r.examples,
              (std::vector<example>{
                  {1}, {2}, {3}, {4}, {-1}, {-2}, {-3}, {-4}}));
}

struct monster
{
    enum color
    {
        red,
        blue,
        green
    };

    struct vec3
    {
        float x;
        float y;
        float z;

        bool operator==(const vec3 &) const = default;
    };

    struct weapon
    {
        std::string name;
        int damage;

        bool operator==(const weapon &) const = default;
    };

    vec3 pos;
    zpp::bits::vint32_t mana;
    int hp;
    std::string name;
    std::vector<std::uint8_t> inventory;
    color color;
    std::vector<weapon> weapons;
    weapon equipped;
    std::vector<vec3> path;
    bool boss;

    bool operator==(const monster &) const = default;
};

TEST(test_pb_protocol2, test_monster)
{
    auto [data, in, out] = zpp::bits::data_in_out(zpp::bits::protobuf{});
    monster m = {.pos = {1.0, 2.0, 3.0},
                 .mana = 200,
                 .hp = 1000,
                 .name = "mushroom",
                 .inventory = {1, 2, 3},
                 .color = monster::color::blue,
                 .weapons =
                     {
                         monster::weapon{.name = "sword", .damage = 55},
                         monster::weapon{.name = "spear", .damage = 150},
                     },
                 .equipped =
                     {
                         monster::weapon{.name = "none", .damage = 15},
                     },
                 .path = {monster::vec3{2.0, 3.0, 4.0},
                          monster::vec3{5.0, 6.0, 7.0}},
                 .boss = true};
    out(m).or_throw();

    monster m2;
    in(m2).or_throw();

    EXPECT_EQ(m.pos, m2.pos);
    EXPECT_EQ(m.mana, m2.mana);
    EXPECT_EQ(m.hp, m2.hp);
    EXPECT_EQ(m.name, m2.name);
    EXPECT_EQ(m.inventory, m2.inventory);
    EXPECT_EQ(m.color, m2.color);
    EXPECT_EQ(m.weapons, m2.weapons);
    EXPECT_EQ(m.equipped, m2.equipped);
    EXPECT_EQ(m.path, m2.path);
    EXPECT_EQ(m.boss, m2.boss);
    EXPECT_EQ(m, m2);
}

TEST(test_pb_protocol2, test_monster_unsized)
{
    auto [data, in, out] = zpp::bits::data_in_out(zpp::bits::no_size{},zpp::bits::protobuf{});
    monster m = {.pos = {1.0, 2.0, 3.0},
                 .mana = 200,
                 .hp = 1000,
                 .name = "mushroom",
                 .inventory = {1, 2, 3},
                 .color = monster::color::blue,
                 .weapons =
                     {
                         monster::weapon{.name = "sword", .damage = 55},
                         monster::weapon{.name = "spear", .damage = 150},
                     },
                 .equipped =
                     {
                         monster::weapon{.name = "none", .damage = 15},
                     },
                 .path = {monster::vec3{2.0, 3.0, 4.0},
                          monster::vec3{5.0, 6.0, 7.0}},
                 .boss = true};
    out(m).or_throw();

    monster m2;
    in(m2).or_throw();

    EXPECT_EQ(m.pos, m2.pos);
    EXPECT_EQ(m.mana, m2.mana);
    EXPECT_EQ(m.hp, m2.hp);
    EXPECT_EQ(m.name, m2.name);
    EXPECT_EQ(m.inventory, m2.inventory);
    EXPECT_EQ(m.color, m2.color);
    EXPECT_EQ(m.weapons, m2.weapons);
    EXPECT_EQ(m.equipped, m2.equipped);
    EXPECT_EQ(m.path, m2.path);
    EXPECT_EQ(m.boss, m2.boss);
    EXPECT_EQ(m, m2);
}

struct person
{
    std::string name; // = 1
    zpp::bits::vint32_t id; // = 2
    std::string email; // = 3

    enum phone_type
    {
        mobile = 0,
        home = 1,
        work = 2,
    };

    struct phone_number
    {
        std::string number; // = 1
        phone_type type; // = 2
    };

    std::vector<phone_number> phones; // = 4
};

struct address_book
{
    std::vector<person> people; // = 1
};


TEST(test_pb_protocol2, person)
{
    constexpr auto data =
        "\n\x08John Doe\x10\xd2\t\x1a\x10jdoe@example.com\"\x0c\n\x08"
        "555-4321\x10\x01"_b;
    static_assert(data.size() == 45);

    person p;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.id, 1234);
    EXPECT_EQ(p.email, "jdoe@example.com");
    ASSERT_EQ(p.phones.size(), 1u);
    EXPECT_EQ(p.phones[0].number, "555-4321");
    EXPECT_EQ(p.phones[0].type, person::home);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(data, new_data);
}

TEST(test_pb_protocol2, address_book)
{
    constexpr auto data =
        "\n-\n\x08John Doe\x10\xd2\t\x1a\x10jdoe@example.com\"\x0c\n\x08"
        "555-4321\x10\x01\n>\n\nJohn Doe "
        "2\x10\xd3\t\x1a\x11jdoe2@example.com\"\x0c\n\x08"
        "555-4322\x10\x01\"\x0c\n\x08"
        "555-4323\x10\x02"_b;

    static_assert(data.size() == 111);

    address_book b;
    zpp::bits::in{data, zpp::bits::no_size{},zpp::bits::protobuf{}}(b).or_throw();

    ASSERT_EQ(b.people.size(), 2u);
    EXPECT_EQ(b.people[0].name, "John Doe");
    EXPECT_EQ(b.people[0].id, 1234);
    EXPECT_EQ(b.people[0].email, "jdoe@example.com");
    ASSERT_EQ(b.people[0].phones.size(), 1u);
    EXPECT_EQ(b.people[0].phones[0].number, "555-4321");
    EXPECT_EQ(b.people[0].phones[0].type, person::home);
    EXPECT_EQ(b.people[1].name, "John Doe 2");
    EXPECT_EQ(b.people[1].id, 1235);
    EXPECT_EQ(b.people[1].email, "jdoe2@example.com");
    ASSERT_EQ(b.people[1].phones.size(), 2u);
    EXPECT_EQ(b.people[1].phones[0].number, "555-4322");
    EXPECT_EQ(b.people[1].phones[0].type, person::home);
    EXPECT_EQ(b.people[1].phones[1].number, "555-4323");
    EXPECT_EQ(b.people[1].phones[1].type, person::work);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out out{new_data, zpp::bits::no_size{},zpp::bits::protobuf{}};
    out(b).or_throw();
    EXPECT_EQ(out.position(), data.size());
    EXPECT_EQ(data, new_data);
}

struct person_explicit
{
    zpp::bits::pb_field<std::string, 10> extra;
    zpp::bits::pb_field<std::string, 1> name;
    zpp::bits::pb_field<zpp::bits::vint32_t, 2> id;
    zpp::bits::pb_field<std::string, 3> email;

    enum phone_type
    {
        mobile = 0,
        home = 1,
        work = 2,
    };

    struct phone_number
    {
        zpp::bits::pb_field<std::string, 1> number;
        zpp::bits::pb_field<phone_type, 2> type;
    };

    zpp::bits::pb_field<std::vector<phone_number>, 4> phones;
};


TEST(test_pb_protocol2, person_explicit)
{
    constexpr auto data =
        "\n\x08John Doe\x10\xd2\t\x1a\x10jdoe@example.com\"\x0c\n\x08"
        "555-4321\x10\x01"_b;
    static_assert(data.size() == 45);

    person_explicit p;
    zpp::bits::in{data, zpp::bits::no_size{},zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.id, 1234);
    EXPECT_EQ(p.email, "jdoe@example.com");
    ASSERT_EQ(p.phones.size(), 1u);
    EXPECT_EQ(p.phones[0].number, "555-4321");
    EXPECT_EQ(p.phones[0].type, person_explicit::home);

    person p1;
    p1.name = p.name;
    p1.id = p.id;
    p1.email = p.email;
    p1.phones.push_back({p.phones[0].number,
                         person::phone_type(pb_value(p.phones[0].type))});

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{},zpp::bits::protobuf{}}(p1).or_throw();

    EXPECT_EQ(data, new_data);
}

struct person_map
{
    std::string name;       // = 1
    zpp::bits::vint32_t id; // = 2
    std::string email;      // = 3

    enum phone_type
    {
        mobile = 0,
        home = 1,
        work = 2,
    };

    std::map<std::string, phone_type> phones; // = 4
};

TEST(test_pb_protocol2, person_map)
{
    constexpr auto data =
        "\n\x08John Doe\x10\xd2\t\x1a\x10jdoe@example.com\"\x0c\n\x08"
        "555-4321\x10\x01"_b;
    static_assert(data.size() == 45);

    person_map p;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.id, 1234);
    EXPECT_EQ(p.email, "jdoe@example.com");
    ASSERT_EQ(p.phones.size(), 1u);
    ASSERT_TRUE(p.phones.contains("555-4321"));
    EXPECT_EQ(p.phones["555-4321"], person_map::home);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(data, new_data);
}

struct person_short
{
    std::string name;       // = 1
    zpp::bits::vint32_t id; // = 2
    std::string email;      // = 3

    enum phone_type
    {
        mobile = 0,
        home = 1,
        work = 2,
    };

    struct phone_number
    {
        std::string number; // = 1
        phone_type type;    // = 2
    };

    std::vector<phone_number> phones; // = 4
};

TEST(test_pb_protocol2, person_short)
{
    constexpr auto data =
        "\n\x08John Doe\x10\xd2\t\x1a\x10jdoe@example.com\"\x0c\n\x08"
        "555-4321\x10\x01"_b;
    static_assert(data.size() == 45);

    person_short p;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.id, 1234);
    EXPECT_EQ(p.email, "jdoe@example.com");
    ASSERT_EQ(p.phones.size(), 1u);
    EXPECT_EQ(p.phones[0].number, "555-4321");
    EXPECT_EQ(p.phones[0].type, person_short::home);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(data, new_data);
}

TEST(test_pb_protocol2, default_person_in_address_book)
{
    constexpr auto data = "\n\x00"_b;

    address_book b;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(b).or_throw();

    EXPECT_EQ(b.people.size(), 1u);
    EXPECT_EQ(b.people[0].name, "");
    EXPECT_EQ(b.people[0].id, 0);
    EXPECT_EQ(b.people[0].email, "");
    EXPECT_EQ(b.people[0].phones.size(), 0u);

    std::array<std::byte, "0a000000"_decode_hex.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(b).or_throw();

    EXPECT_EQ(new_data, "0a000000"_decode_hex);
}

TEST(test_pb_protocol2, empty_address_book)
{
    constexpr auto data = ""_b;

    address_book b;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(b).or_throw();

    EXPECT_EQ(b.people.size(), 0u);

    std::array<std::byte, 1> new_data;
    zpp::bits::out out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
    out(b).or_throw();

    EXPECT_EQ(out.position(), 0u);
}

TEST(test_pb_protocol2, empty_person)
{
    constexpr auto data = ""_b;

    person p;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(p).or_throw();

    EXPECT_EQ(p.name.size(), 0u);
    EXPECT_EQ(p.name, "");
    EXPECT_EQ(p.id, 0);
    EXPECT_EQ(p.email, "");
    EXPECT_EQ(p.phones.size(), 0u);

    std::array<std::byte, 2> new_data;
    zpp::bits::out out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
    out(p).or_throw();
    EXPECT_EQ(out.position(), 0u);
}

struct monster_with_optional
{
    using serialize = zpp::bits::members<10>;
    enum class color : int
    {
        red,
        blue,
        green
    };

    struct vec3
    {
        int32_t x;
        int32_t y;
        int32_t z;

        bool operator==(const vec3&) const = default;
    };

    struct weapon
    {
        std::string name;
        int32_t damage;

        bool operator==(const weapon&) const = default;
    };

    vec3 pos;
    int32_t mana;
    int64_t hp;
    std::string name;
    std::vector<std::uint8_t> inventory;
    color color;
    std::vector<weapon> weapons;
    std::optional<weapon> equipped;
    std::vector<vec3> path;
    bool boss;

    bool operator==(const monster_with_optional&) const = default;
};

TEST(test_pb_protocol2, monster_with_optional)
{
    constexpr auto data =
        "0a0f0d0100000015020000001d0300000015c800000019e80300000000000030015001"_decode_hex;

    monster_with_optional m;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(m).or_throw();

    EXPECT_EQ(m.pos.x, 1);
    EXPECT_EQ(m.pos.y, 2);
    EXPECT_EQ(m.pos.z, 3);
    EXPECT_EQ(m.mana, 200);
    EXPECT_EQ(m.hp, 1000);
    EXPECT_EQ(m.name.size(), 0u);
    EXPECT_EQ(m.inventory.size(), 0u);
    EXPECT_EQ(m.color, monster_with_optional::color::blue);
    EXPECT_EQ(m.weapons.size(), 0u);
    EXPECT_EQ(m.equipped.has_value(), false);
    EXPECT_EQ(m.path.size(), 0u);
    EXPECT_EQ(m.boss, true);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(m).or_throw();

    EXPECT_EQ(data, new_data);
}

struct recursive_example
{
    zpp::bits::vint32_t i;
    std::unique_ptr<recursive_example> nested;

    bool operator==(const recursive_example& other) const
    {
        return i == other.i && ((!nested.get() && !other.nested.get()) || *nested == *other.nested);
    }
};

TEST(test_pb_protocol2, recursive_nested_empty)
{
    constexpr auto data = "089601"_decode_hex;
    recursive_example x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.i, 150);
    EXPECT_EQ(x.nested.get(), nullptr);


    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}

TEST(test_pb_protocol2, recursive_nested_nonempty)
{
    constexpr auto data = "08960112020802"_decode_hex;
    recursive_example x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.i, 150);
    EXPECT_EQ(x.nested->i, 2);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}

struct reservied_field_example_v0
{
    zpp::bits::vint32_t i; // field number == 1
    [[no_unique_address]] zpp::bits::pb_reserved _2{};
    [[no_unique_address]] zpp::bits::pb_reserved _3{};
    [[no_unique_address]] zpp::bits::pb_reserved _4{};
    [[no_unique_address]] zpp::bits::pb_reserved _5{};
    zpp::bits::vsint32_t j;
    // using pb_options = std::tuple<zpp::bits::pb_map<2, 6>>;
};

struct reservied_field_example_v1
{
    zpp::bits::vint32_t i;                            // field number == 1
    std::optional<std::uint64_t> field_64;            // field number == 2
    std::optional<std::uint32_t> field_32;            // field number == 3
    std::optional<example> field_length_delimited;    // field number == 4
    std::optional<zpp::bits::vuint32_t> field_varint; // field number == 5
    zpp::bits::vsint32_t j;
};

TEST(test_pb_protocol2, reserved_field)
{
    auto test_case = [](auto origin) {
        std::vector<std::byte> data;
        zpp::bits::out out{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
        EXPECT_EQ(out(origin), std::errc{});

        zpp::bits::in in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};

        reservied_field_example_v0 restored;
        EXPECT_EQ(in(restored), std::errc{});
        EXPECT_EQ(restored.i, origin.i);
        EXPECT_EQ(restored.j, origin.j);
    };

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = 2,
            .field_32 = {},
            .field_length_delimited = {},
            .field_varint = {},
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = 2,
            .field_length_delimited = {},
            .field_varint = {},
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = {},
            .field_length_delimited = {},
            .field_varint = 256,
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = {},
            .field_length_delimited = example{2},
            .field_varint = {},
            .j = 3});
}

struct unknown_field_example_v0
{
    zpp::bits::vint32_t i; // field number == 1
    zpp::bits::vsint32_t j;
    using pb_options = std::tuple<zpp::bits::pb_map<2, 6>>;
};

TEST(test_pb_protocol2, unknown_field)
{
    auto test_case = [](auto origin) {
        std::vector<std::byte> data;
        zpp::bits::out out{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
        EXPECT_EQ(out(origin), std::errc{});

        zpp::bits::in in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};

        unknown_field_example_v0 restored;
        EXPECT_EQ(in(restored), std::errc{});
        EXPECT_EQ(restored.i, origin.i);
        EXPECT_EQ(restored.j, origin.j);
    };

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = 2,
            .field_32 = {},
            .field_length_delimited = {},
            .field_varint = {},
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = 2,
            .field_length_delimited = {},
            .field_varint = {},
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = {},
            .field_length_delimited = {},
            .field_varint = 256,
            .j = 3});

    test_case(
        reservied_field_example_v1{
            .i = 1,
            .field_64 = {},
            .field_32 = {},
            .field_length_delimited = example{2},
            .field_varint = {},
            .j = 3});
}

struct example_with_nested_serailize
{
    struct mapped_data
    {
        mapped_data() = default;
        uint64_t value;
        constexpr static auto pb_serialize(auto& archive, auto& self)
        {
            return archive(self.value);
        }
        bool operator==(const mapped_data&) const = default;
    } data;
    bool operator==(const example_with_nested_serailize&) const = default;
};


TEST(test_pb_protocol2, example_with_nested_serailize)
{
    constexpr auto data = "090200000000000000"_decode_hex;
    example_with_nested_serailize x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.data.value, 2ull);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}


struct example_with_free_serailize
{
    struct mapped_data
    {
        mapped_data() = default;
        uint64_t value;
        bool operator==(const mapped_data&) const = default;
    } data;
    bool operator==(const example_with_free_serailize&) const = default;
};

constexpr auto pb_serialize(auto& archive, const example_with_free_serailize::mapped_data& self)
{
    return archive(self.value);
}

constexpr auto pb_serialize(auto& archive, example_with_free_serailize::mapped_data& self)
{
    return archive(self.value);
}

TEST(test_pb_protocol2, free_serailize)
{
    constexpr auto data = "090200000000000000"_decode_hex;
    example_with_free_serailize x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.data.value, 2ull);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}


struct example_with_nested_serailize2
{
    struct nested_data
    {
        uint64_t f1;
        std::string f2;
        nested_data() = default;
        
        constexpr static auto pb_serialize(auto& archive, auto& self)
        {
            return archive(self.f1, self.f2);
        }
        bool operator==(const nested_data&) const = default;
        using pb_options = std::tuple< zpp::bits::pb_map<1, 11>, zpp::bits::pb_map<2, 12>>;

    } data;
    zpp::bits::vint32_t data2;

    bool operator==(const example_with_nested_serailize2&) const = default;

    using pb_options = std::tuple< zpp::bits::pb_map<1, 3>, zpp::bits::pb_map<2, 4> >;
};


TEST(test_pb_protocol2, example_with_nested_serailize2)
{
    constexpr auto data = "1a0e597b0000000000000062036162632002"_decode_hex;
    example_with_nested_serailize2 x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.data.f1, 123U);
    EXPECT_EQ(x.data.f2, "abc");
    EXPECT_EQ(x.data2, 2);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}


struct example_with_serailize
{
    example_with_serailize() = default;
    struct mapped_data
    {
        mapped_data() = default;
        uint64_t value;
        bool operator==(const mapped_data&) const = default;
    } data;
    uint32_t extra = 0;

    constexpr static auto pb_serialize(auto&& archive, auto& self)
    {
        return archive(self.data.value);
    }

    bool operator==(const example_with_serailize&) const = default;
};



TEST(test_pb_protocol2, example_with_serailize)
{
    constexpr auto data = "090200000000000000"_decode_hex;
    example_with_serailize x;
    zpp::bits::in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();
    EXPECT_EQ(x.data.value, 2ull);

    std::array<std::byte, data.size()> new_data;
    zpp::bits::out{new_data, zpp::bits::no_size{}, zpp::bits::protobuf{}}(x).or_throw();

    EXPECT_EQ(data, new_data);
}

struct custom_convertion_field1
{
    unsigned long x;

    static auto pb_serialize(auto&& archive, const custom_convertion_field1& v)
    {
        return archive(std::to_string(v.x));
    }

    static auto pb_serialize(auto&& archive, custom_convertion_field1& v)
    {
        auto remaining_data = archive.remaining_data();
        v.x = std::stoul(std::string((const char*)remaining_data.data(), remaining_data.size()));
        return std::errc{};
    }
};

struct message_with_custom_convertion_field1
{
    custom_convertion_field1 f;
};


TEST(test_pb_protocol2, custom_convertion_example1)
{
    message_with_custom_convertion_field1 origin;
    origin.f.x = 1000UL;
    std::vector<std::byte> data;
    zpp::bits::out out{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
    EXPECT_EQ(out(origin), std::errc{});

    zpp::bits::in in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};

    message_with_custom_convertion_field1 restored;
    EXPECT_EQ(in(restored), std::errc{});
    EXPECT_EQ(restored.f.x, 1000UL);
}

struct custom_convertion_field2
{
    unsigned long x;
};

auto pb_serialize(auto && archive, const custom_convertion_field2& v)
{
    return archive(std::to_string(v.x));
}

auto pb_serialize(auto && archive, custom_convertion_field2& v)
{
    auto remaining_data = archive.remaining_data();
    v.x = std::stoul(std::string((const char*)remaining_data.data(), remaining_data.size()));
    return std::errc{};
}

struct message_with_custom_convertion_field2
{
    custom_convertion_field2 f;
};


TEST(test_pb_protocol2, custom_convertion_example2)
{
    message_with_custom_convertion_field2 origin;
    origin.f.x = 1000UL;
    std::vector<std::byte> data;
    zpp::bits::out out{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};
    EXPECT_EQ(out(origin), std::errc{});

    zpp::bits::in in{data, zpp::bits::no_size{}, zpp::bits::protobuf{}};

    message_with_custom_convertion_field2 restored;
    EXPECT_EQ(in(restored), std::errc{});
    EXPECT_EQ(restored.f.x, 1000UL);
}

} // namespace test_pb_protocol2
