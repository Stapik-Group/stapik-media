#pragma once

#include "stapik/document/SchemaMigrator.hpp"

inline constexpr int MEDIA_SCHEMA_VERSION = 2;

[[nodiscard]] stapik::document::SchemaMigrator createMediaSchemaMigrator();
