#pragma once

#include <string>
#include <vector>
#include "TrackerTypes.h"

namespace JsonHelper
{
    inline std::string Escape(const std::string& a_str)
    {
        std::string out;
        out.reserve(a_str.size() + 10);
        for (char c : a_str) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
                break;
            }
        }
        return out;
    }

    inline std::string SerializeRecipes(const std::vector<TrackedRecipe>& a_recipes)
    {
        std::string json = "[";
        bool firstRecipe = true;

        for (const auto& recipe : a_recipes) {
            if (!firstRecipe) json += ",";
            firstRecipe = false;

            json += "{";
            json += "\"cobjFormID\":" + std::to_string(recipe.cobjFormID) + ",";
            json += "\"createdFormID\":" + std::to_string(recipe.createdFormID) + ",";
            json += "\"itemName\":\"" + Escape(recipe.itemName) + "\",";
            json += "\"countProduced\":" + std::to_string(recipe.countProduced) + ",";
            json += "\"multiplier\":" + std::to_string(recipe.multiplier) + ",";
            json += "\"isReady\":" + std::string(recipe.isReady ? "true" : "false") + ",";
            json += "\"ingredients\":[";

            bool firstIng = true;
            for (const auto& ing : recipe.ingredients) {
                if (!firstIng) json += ",";
                firstIng = false;

                json += "{";
                json += "\"formID\":" + std::to_string(ing.formID) + ",";
                json += "\"name\":\"" + Escape(ing.name) + "\",";
                json += "\"need\":" + std::to_string(ing.need) + ",";
                json += "\"have\":" + std::to_string(ing.have) + ",";
                json += "\"missing\":" + std::to_string(ing.missing);
                json += "}";
            }
            json += "]}";
        }

        json += "]";
        return json;
    }
}
