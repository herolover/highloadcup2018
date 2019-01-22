#pragma once

#include <variant>
#include <string_view>

using namespace std::literals;

struct invalid
{
    constexpr static std::string_view name = ""sv;
};

struct f_id
{
    constexpr static std::string_view name = "id"sv;
};

struct f_sex
{
    constexpr static std::string_view name = "sex"sv;
};

struct f_email
{
    constexpr static std::string_view name = "email"sv;
};

struct f_status
{
    constexpr static std::string_view name = "status"sv;
};

struct f_first_name
{
    constexpr static std::string_view name = "fname"sv;
};

struct f_second_name
{
    constexpr static std::string_view name = "sname"sv;
};

struct f_phone
{
    constexpr static std::string_view name = "phone"sv;
};

struct f_country
{
    constexpr static std::string_view name = "country"sv;
};

struct f_city
{
    constexpr static std::string_view name = "city"sv;
};

struct f_birth
{
    constexpr static std::string_view name = "birth"sv;
};

struct f_joined
{
    constexpr static std::string_view name = "joined"sv;
};

struct f_interests
{
    constexpr static std::string_view name = "interests"sv;
};

struct f_likes
{
    constexpr static std::string_view name = "likes"sv;
};

struct f_premium
{
    constexpr static std::string_view name = "premium"sv;
};


struct m_eq {};
struct m_neq {};
struct m_any {};
struct m_contains {};
struct m_null {};
struct m_lt {};
struct m_gt {};
struct m_starts {};
struct m_domain {};
struct m_code {};
struct m_year {};
struct m_now {};

// *INDENT-OFF*
using Field = std::variant<
    invalid,
    f_id,
    f_sex,
    f_email,
    f_status,
    f_first_name,
    f_second_name,
    f_phone,
    f_country,
    f_city,
    f_birth,
    f_joined,
    f_interests,
    f_likes,
    f_premium>;

using Method = std::variant<
    invalid,
    m_eq,
    m_neq,
    m_any,
    m_contains,
    m_null,
    m_lt,
    m_gt,
    m_starts,
    m_domain,
    m_code,
    m_year,
    m_now>;

using Value = std::variant<
    std::string_view,
    std::vector<std::string_view>,
    std::pair<std::vector<std::string_view>, Account::interest_mask_t>,
    std::pair<std::string_view, Account::interest_mask_t>,
    std::vector<uint32_t>,
    std::nullptr_t,
    uint32_t,
    uint16_t,
    int32_t,
    Account::Status,
    Account::Sex,
    bool>;
// *INDENT-ON*

inline Field make_field(const std::string_view &field_name)
{
    Field field;
    if (field_name == "birth"sv)
    {
        field = f_birth();
    }
    else if (field_name == "city"sv)
    {
        field = f_city();
    }
    else if (field_name == "country"sv)
    {
        field = f_country();
    }
    else if (field_name == "email"sv)
    {
        field = f_email();
    }
    else if (field_name == "fname"sv)
    {
        field = f_first_name();
    }
    else if (field_name == "interests"sv)
    {
        field = f_interests();
    }
    else if (field_name == "joined"sv)
    {
        field = f_joined();
    }
    else if (field_name == "likes"sv)
    {
        field = f_likes();
    }
    else if (field_name == "phone"sv)
    {
        field = f_phone();
    }
    else if (field_name == "premium"sv)
    {
        field = f_premium();
    }
    else if (field_name == "sex"sv)
    {
        field = f_sex();
    }
    else if (field_name == "sname"sv)
    {
        field = f_second_name();
    }
    else if (field_name == "status"sv)
    {
        field = f_status();
    }

    return field;
}

inline Method make_method(const std::string_view &method_name)
{
    Method method;
    if (method_name == "eq"sv)
    {
        method = m_eq();
    }
    else if (method_name == "neq"sv)
    {
        method = m_neq();
    }
    else if (method_name == "any"sv)
    {
        method = m_any();
    }
    else if (method_name == "contains"sv)
    {
        method = m_contains();
    }
    else if (method_name == "null"sv)
    {
        method = m_null();
    }
    else if (method_name == "lt"sv)
    {
        method = m_lt();
    }
    else if (method_name == "gt"sv)
    {
        method = m_gt();
    }
    else if (method_name == "starts"sv)
    {
        method = m_starts();
    }
    else if (method_name == "domain"sv)
    {
        method = m_domain();
    }
    else if (method_name == "code"sv)
    {
        method = m_code();
    }
    else if (method_name == "year"sv)
    {
        method = m_year();
    }
    else if (method_name == "now"sv)
    {
        method = m_now();
    }

    return method;
}
