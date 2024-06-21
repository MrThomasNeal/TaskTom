#include "MyBot.h"
#include <dpp/dpp.h>

const std::string BOT_TOKEN = "";

int main()
{
	// Create bot cluster
	dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content);

	// Output simple log messages to stdout
	bot.on_log(dpp::utility::cout_logger());

	// Register slash command here in on_ready 
	bot.on_ready([&bot](const dpp::ready_t& event) {
		// Wrap command registration in run_once to make sure it doesnt run on every full reconnection 
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands {
				{ "ping", "Ping pong!", bot.me.id }
			};

			bot.global_bulk_command_create(commands);
		}
	});

	bot.on_guild_member_add([&bot](const dpp::guild_member_add_t& event) {

		// Channel ID for welcome message to be displayed in
		dpp::snowflake welcome_channel_id = 0;

		dpp::snowflake guild_id = event.added.guild_id;
		std::vector<dpp::snowflake> channels = event.adding_guild->channels;

		for (const auto& channel : channels) {
			if (channel) {
				dpp::channel* tempChannel = dpp::find_channel(channel.get_process_id());
				if (tempChannel) {
					std::cout << tempChannel->name << std::endl;
					if (tempChannel->name == "general") {
						welcome_channel_id = tempChannel->id;
					}
				}
			}
		}

		// Create the welcome message
		std::string welcome_message = "Welcome to the server, " + event.added.get_user()->format_username() + "! Nice to see you :)";

		// Send the welcome message to the specified channel
		if (welcome_channel_id != 0) {
			bot.message_create(dpp::message(welcome_channel_id, welcome_message));
		}
		else {
			std::cout << "ERROR: No 'general' channel found" << std::endl;
		}

	});

	bot.on_message_create([&bot](const dpp::message_create_t& event) {

		// Simple testing to initially see if bot worked
		if (event.msg.content == "testing") {
			bot.message_create(dpp::message(event.msg.channel_id, "1, 2!"));
		}

	});

	// Handle slash command with the most recent addition to D++ features, coroutines! 
	bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (event.command.get_command_name() == "ping") {
			co_await event.co_reply("Pong!");
		}
		co_return;
	});

	// Start the bot 
	bot.start(dpp::st_wait);

	return 0;
}
