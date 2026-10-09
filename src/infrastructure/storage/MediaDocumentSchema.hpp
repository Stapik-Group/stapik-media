#pragma once

#include "stapik/document/SchemaMigrator.hpp"

inline constexpr int MEDIA_SCHEMA_VERSION = 1;

[[nodiscard]] stapik::document::SchemaMigrator createMediaSchemaMigrator();
