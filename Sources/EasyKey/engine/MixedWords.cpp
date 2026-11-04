//
//  MixedWords.cpp
//  EasyKey
//  Copyright (C) 2026 Cino
//  SPDX-License-Identifier: GPL-3.0-only
//

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include "MixedWords.h"
#include "MixedWordsData.h"

using namespace std;

static unordered_map<string, unsigned short> _mixedWords;

// Sorted curated AI/LLM/developer/product proper nouns that must stay English in Mixed mode.
// Generic short ambiguous tokens ("bus", "us", "too", "car", "or") are intentionally excluded
// and handled via collocation phrases and short-Vietnamese protection.
static const char* const kCuratedSafeEnglish[] = {
    "anthropic",
    "autogen",
    "bedrock",
    "chatgpt",
    "claude",
    "clion",
    "codeforces",
    "codeium",
    "codewars",
    "codex",
    "colab",
    "copilot",
    "crewai",
    "cuda",
    "cudnn",
    "cursor",
    "cypress",
    "databricks",
    "datagrip",
    "davinci",
    "deepseek",
    "deepseekcoder",
    "deepseekmath",
    "discord",
    "docker",
    "dotnet",
    "duckdb",
    "elasticsearch",
    "elevenlabs",
    "esbuild",
    "eslint",
    "esp",
    "fastjson",
    "firebase",
    "flutter",
    "foobar",
    "gemini",
    "genshin",
    "github",
    "githubcopilot",
    "gitlab",
    "goland",
    "golang",
    "grafana",
    "graphql",
    "grok",
    "grpc",
    "hackerrank",
    "hadoop",
    "haystack",
    "heroku",
    "honkai",
    "huggingface",
    "insomnia",
    "instagram",
    "intellij",
    "istio",
    "jaeger",
    "javascript",
    "jetbrains",
    "jupyter",
    "kafka",
    "kaggle",
    "kibana",
    "kling",
    "kotlin",
    "kubernetes",
    "langchain",
    "leetcode",
    "llama",
    "llamaindex",
    "luma",
    "mariadb",
    "memcached",
    "midjourney",
    "minecraft",
    "mistral",
    "mixtral",
    "mlflow",
    "mongodb",
    "mysql",
    "nccl",
    "netlify",
    "nginx",
    "nodejs",
    "ollama",
    "onnx",
    "openai",
    "openrouter",
    "opentelemetry",
    "perplexity",
    "phpstorm",
    "pika",
    "planetscale",
    "playwright",
    "pnpm",
    "postgres",
    "postgresql",
    "postman",
    "powertoys",
    "prefect",
    "prettier",
    "prometheus",
    "protobuf",
    "pulsar",
    "pycharm",
    "python",
    "pytorch",
    "qwen",
    "rabbitmq",
    "rainmeter",
    "ray",
    "reddit",
    "redis",
    "redshift",
    "replicate",
    "resharper",
    "restapi",
    "rider",
    "roblox",
    "rollup",
    "runway",
    "rust",
    "sagemaker",
    "selenium",
    "semantickernel",
    "serverfault",
    "snowflake",
    "spotify",
    "sqlite",
    "stablediffusion",
    "stackoverflow",
    "starrail",
    "suno",
    "supabase",
    "superuser",
    "swift",
    "tabnine",
    "telegram",
    "tensorboard",
    "tensorflow",
    "tiktok",
    "tmux",
    "togetherai",
    "toml",
    "transformers",
    "triton",
    "twitter",
    "typescript",
    "udio",
    "valorant",
    "vercel",
    "vertex",
    "vite",
    "vitest",
    "vllm",
    "vscode",
    "wandb",
    "webex",
    "webpack",
    "websocket",
    "webstorm",
    "wezterm",
    "whatsapp",
    "whisper",
    "windsurf",
    "wutheringwaves",
    "xai",
    "yaml",
    "yarn",
    "youtube",
    "zellij",
    "zoom",
};

// Sorted ALL-CAPS English acronyms (2 to 4 letters) that must stay English when typed in all uppercase,
// while allowing lowercase forms ("us" -> "ú", "ai" -> "ai", "er" -> "ẻ", "as" -> "á") to follow Vietnamese rules.
// Excludes Vietnamese uppercase abbreviations ("DDT", "DDHQG").
static const char* const kAllCapsAcronyms[] = {
    "ai",
    "an",
    "api",
    "as",
    "at",
    "aws",
    "by",
    "ceo",
    "cia",
    "cli",
    "cpu",
    "css",
    "csv",
    "cto",
    "db",
    "dns",
    "dom",
    "er",
    "eu",
    "faq",
    "fbi",
    "fps",
    "ftp",
    "gcp",
    "gdp",
    "gif",
    "gpu",
    "gui",
    "hr",
    "html",
    "http",
    "id",
    "ide",
    "if",
    "in",
    "io",
    "ios",
    "ip",
    "is",
    "iso",
    "it",
    "jpg",
    "js",
    "json",
    "jwt",
    "kda",
    "lan",
    "llm",
    "lol",
    "mac",
    "ml",
    "mmr",
    "mvp",
    "my",
    "nat",
    "no",
    "npc",
    "npm",
    "ocr",
    "oem",
    "ok",
    "on",
    "oop",
    "or",
    "orm",
    "os",
    "otp",
    "pc",
    "pdf",
    "php",
    "pin",
    "png",
    "pr",
    "pve",
    "pvp",
    "qa",
    "qr",
    "ram",
    "rgb",
    "rom",
    "rpc",
    "rpg",
    "rss",
    "rt",
    "rts",
    "sdk",
    "seo",
    "sim",
    "sla",
    "sms",
    "so",
    "sql",
    "ssd",
    "ssh",
    "ssl",
    "sso",
    "svg",
    "tcp",
    "tls",
    "to",
    "ttl",
    "tv",
    "udp",
    "ui",
    "uk",
    "un",
    "up",
    "uri",
    "url",
    "us",
    "usa",
    "usb",
    "utc",
    "utf",
    "ux",
    "vip",
    "vm",
    "vpn",
    "vram",
    "wan",
    "web",
    "wifi",
    "win",
    "wsl",
    "www",
    "xml",
    "zip",
};
void parseMixedWords(const char* data, unordered_map<string, unsigned short>& out) {
    const char* p = data;
    while (*p) {
        const char* end = p;
        while (*end && *end != '\n') end++;
        if (!*end) break; //last line has no newline: drop it, never scan past the end (finding F6)
        if (end - p > 2)
            out[string(p, end - p - 2)] = (unsigned short)(((end[-2] - '0') << 8) | (end[-1] - '0'));
        p = end + 1;
    }
}

static void initMixedWords() {
    _mixedWords.reserve(MIXED_WORD_COUNT);
    parseMixedWords(MIXED_WORDS, _mixedWords);
}

bool findMixedWord(const string& keys, int& english, int& vietnamese) {
    if (_mixedWords.empty())
        initMixedWords();
    auto it = _mixedWords.find(keys);
    if (it == _mixedWords.end())
        return false;
    english = it->second >> 8;
    vietnamese = it->second & 0xFF;
    return true;
}

bool isCuratedSafeEnglish(const string& keys) {
    if (keys.empty())
        return false;
    const size_t count = sizeof(kCuratedSafeEnglish) / sizeof(kCuratedSafeEnglish[0]);
    return std::binary_search(kCuratedSafeEnglish, kCuratedSafeEnglish + count, keys.c_str(),
        [](const char* a, const char* b) {
            return std::strcmp(a, b) < 0;
        });
}

bool isAllCapsEnglishAcronym(const string& keys, bool isAllCaps) {
    if (!isAllCaps || keys.length() < 2 || keys.length() > 4)
        return false;
    const size_t count = sizeof(kAllCapsAcronyms) / sizeof(kAllCapsAcronyms[0]);
    return std::binary_search(kAllCapsAcronyms, kAllCapsAcronyms + count, keys.c_str(),
        [](const char* a, const char* b) {
            return std::strcmp(a, b) < 0;
        });
}

static bool matchesAnyPrev(const char* prevRaw, uint8_t prevLen, const char* const* list, size_t count) {
    if (!prevRaw || prevLen == 0)
        return false;
    for (size_t i = 0; i < count; i++) {
        if (std::strlen(list[i]) == prevLen && std::memcmp(prevRaw, list[i], prevLen) == 0)
            return true;
    }
    return false;
}

int checkMixedPhrase(const char* prevRaw, uint8_t prevLen, const string& currentRaw) {
    if (!prevRaw || prevLen == 0 || currentRaw.empty())
        return 0;

    // 1. "test" / "tets" preceded by "bánh" / "thịt" -> Vietnamese ("bánh tét", "thịt tét"),
    //    or "test" / "tets" followed by "đít" ("ddits", "ddit")
    if (currentRaw == "test" || currentRaw == "tets") {
        static const char* const kBanhThitPrev[] = {
            "banh", "banhs", "basnh", "banh1",
            "thit", "thijt", "thitj", "thit5"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kBanhThitPrev, sizeof(kBanhThitPrev) / sizeof(kBanhThitPrev[0])))
            return -1;
    }

    // 2. "bus" collocations ("xe bus", "tuyến bus", "trạm bus", "chuyến bus", "đón bus", "bắt bus", "đi bus") -> English
    if (currentRaw == "bus") {
        static const char* const kBusEnglishPrev[] = {
            "xe",
            "tuyen", "tuyeesn", "tuyens", "tuyen1", "tuyeenf",
            "tram", "tramj", "trajm", "tram5",
            "chuyen", "chuyeesn", "chuyens", "chuyen1",
            "don", "ddons", "ddosn", "dons", "ddon1",
            "bat", "bawts", "bats", "bawst", "bat1",
            "di", "ddi",
            "the", "a", "by", "on", "city", "school", "shuttle", "tour", "night", "local", "express", "mini"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kBusEnglishPrev, sizeof(kBusEnglishPrev) / sizeof(kBusEnglishPrev[0])))
            return 1;
    }

    // 3. "us" collocations:
    //    - Vietnamese ("mập ú", "béo ú", "tròn ú", "cậu ú", "bé ú", "con ú", "ú ớ", "ú ụ") -> -1
    //    - English ("with us", "for us", "let us", "tell us", "give us", "contact us", "about us", "join us", ...) -> +1
    if (currentRaw == "us") {
        static const char* const kUsVietPrev[] = {
            "map", "maapj", "majp", "mapj", "maap5", "map5",
            "beo", "beos", "beso", "beo1",
            "tron", "tronf", "trofn", "tron2",
            "cau", "caauj", "cauj", "caau5",
            "be", "bes", "be1",
            "con", "thang", "thawngf", "thangf", "dua", "dduwas", "duas"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kUsVietPrev, sizeof(kUsVietPrev) / sizeof(kUsVietPrev[0])))
            return -1;

        static const char* const kUsEnglishPrev[] = {
            "with", "for", "let", "tell", "give", "contact", "about", "join",
            "follow", "message", "email", "invite", "support", "help", "find",
            "see", "hear", "know", "love", "need", "want", "choose", "pick",
            "select", "to", "from", "between", "among", "around", "behind",
            "before", "after", "like", "call", "ask", "show", "send", "bring",
            "teach", "allow", "make", "keep", "leave", "take"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kUsEnglishPrev, sizeof(kUsEnglishPrev) / sizeof(kUsEnglishPrev[0])))
            return 1;
    }

    // 4. "too" collocations:
    //    - Vietnamese ("một tô", "cái tô", "sơn tô") -> -1
    //    - English ("me too", "you too", "not too", "way too", "far too") -> +1
    if (currentRaw == "too") {
        static const char* const kTooVietPrev[] = {
            "mot", "mootj", "moojt", "motj", "moot5", "mot5",
            "cai", "cais", "caif", "cai1",
            "son", "sown", "son7",
            "hai", "ba", "bon", "boons", "nam", "nawm"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kTooVietPrev, sizeof(kTooVietPrev) / sizeof(kTooVietPrev[0])))
            return -1;

        static const char* const kTooEnglishPrev[] = {
            "me", "you", "not", "it", "that", "he", "she", "they", "we",
            "was", "is", "are", "be", "way", "far"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kTooEnglishPrev, sizeof(kTooEnglishPrev) / sizeof(kTooEnglishPrev[0])))
            return 1;
    }

    // 5. "er" collocations:
    //    - Vietnamese ("đi ẻ", "mắc ẻ", "đau bụng ẻ", "muốn ẻ", "buồn ẻ") -> -1
    if (currentRaw == "er") {
        static const char* const kErVietPrev[] = {
            "di", "ddi",
            "mac", "mawcs", "macs",
            "muon", "muoons", "muons",
            "buon", "buoonf", "buonf",
            "bung", "bungj"
        };
        if (matchesAnyPrev(prevRaw, prevLen, kErVietPrev, sizeof(kErVietPrev) / sizeof(kErVietPrev[0])))
            return -1;
    }

    return 0;
}

bool isProtectedShortVietnamese(const string& keys, int english, int vietnamese, int contextBefore) {
    // Only protect in standalone (context == 0) or Vietnamese/weak context (contextBefore <= 20).
    // When already inside an established English sentence (contextBefore > 20), let normal scoring apply.
    if (contextBefore > 20)
        return false;

    // Curated colloquial/euphemistic short Vietnamese words that may have low formal corpus frequency
    // (e.g. "er" -> "ẻ", colloquial euphemism in everyday Vietnamese chat)
    static const char* const kCuratedProtectedShortViet[] = {
        "bus",
        "er",
        "too",
        "us",
    };
    const size_t count = sizeof(kCuratedProtectedShortViet) / sizeof(kCuratedProtectedShortViet[0]);
    if (std::binary_search(kCuratedProtectedShortViet, kCuratedProtectedShortViet + count, keys.c_str(),
        [](const char* a, const char* b) {
            return std::strcmp(a, b) < 0;
        })) {
        return true;
    }

    (void)english;
    (void)vietnamese;
    return false;
}
