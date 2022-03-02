#include <Application.h>
#include <clmUtil/clm_system.h>
#include <iostream>
#include <exception>
#include <array>
#include <clmMath/clm_matrix.h>

//int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
int main(){
	// Testing
	constexpr clm::math::Matrix<2, std::int32_t> matrixDetTest2x2 = { { 1, 2 },
																	{ 3, 4 } };
	static_assert(matrixDetTest2x2.determinant() == -2);

	constexpr clm::math::Matrix<3, std::int32_t> matrixDetTest3x3 = { {3, 0, 1},
																	  {2, 1, 0},
																	  {0, 3, 2} };
	static_assert(matrixDetTest3x3.determinant() == 12, "The determinant of the 3x3 matrix is incorrect.");

	int ret = 0;
	try
	{
		clm::Application ab(L"Application", 800, 600);
		ret = ab.run();
	}
	catch (const std::exception& e) {
		OutputDebugStringA(std::format("Exception: {}\n", e.what()).c_str());
	}

	return ret;
}