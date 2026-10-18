#include "bs/app/TextKeys.h"

namespace bs {
namespace app {

const char* keyName(TextKey key) {
    switch (key) {
#define BS_TEXT_KEY_NAME(id, name)                                                                \
    case TextKey::id:                                                                             \
        return name;
        BS_TEXT_KEYS(BS_TEXT_KEY_NAME)
#undef BS_TEXT_KEY_NAME
    }
    return "";
}

std::vector<TextKey> allTextKeys() {
    return {
#define BS_TEXT_KEY_LIST(id, name) TextKey::id,
        BS_TEXT_KEYS(BS_TEXT_KEY_LIST)
#undef BS_TEXT_KEY_LIST
    };
}

} // namespace app
} // namespace bs
