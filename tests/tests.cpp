#include <sample/sample.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("mul test")
{
    const auto expected = doctest::Approx(7.68f).epsilon(0.01f);

    const auto actual = sample_mul(3.2f, 2.4f);

    CHECK(expected == actual);
}

TEST_CASE("object test")
{
    const auto expected = doctest::Approx(10.0f).epsilon(0.01f);

    sample_vehicle_t vehicle = sample_vehicle_create("test");
    CHECK(vehicle != nullptr);

    sample_vector_t first{1.0f, 2.0f, 3.0f};
    sample_vehicle_set_velocity(vehicle, &first);

    sample_vector_t second{3.0f, 2.0f, 1.0f};
    const auto actual = sample_vehicle_dot_velocity(vehicle, &second);

    sample_vehicle_destroy(vehicle);

    CHECK(expected == actual);
}
