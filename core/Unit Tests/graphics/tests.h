#pragma once
namespace Graphics {

	void Tests() {
		TEST_CASE("some test", "[Graphics]")
			->Then("something should happen")
			->REQUIRE(5 == 5);

		TEST_CASE("some test", "[Graphics]")
			->Then("something should happen")
			->REQUIRE(6 == 5);

	}

}
#define GRAPHICS_TESTS Graphics::Tests();