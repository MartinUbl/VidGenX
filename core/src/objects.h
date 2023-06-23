#pragma once

#include <memory>
#include <optional>
#include <blend2d.h>
#include <stdexcept>

#include "parser_entities.h"

// here we include all the entity header files in one place
#include "entities/shared.h"
#include "entities/animate.h"
#include "entities/circle.h"
#include "entities/composite.h"
#include "entities/rectangle.h"
#include "entities/wait.h"
