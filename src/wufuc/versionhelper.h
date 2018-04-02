#pragma once

int ver_compare_product_version(VS_FIXEDFILEINFO *pffi, WORD wMajor, WORD wMinor, WORD wBuild, WORD wRev);
bool ver_verify_version_info(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);
