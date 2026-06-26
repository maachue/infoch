#ifndef INFOCH_CLI_OVERRIDE_H
#define INFOCH_CLI_OVERRIDE_H

namespace settings {
struct Settings;
} // namespace settings

namespace cli {
struct Cli;

void cli_override(settings::Settings &set, Cli const &cli);
} // namespace cli

#endif