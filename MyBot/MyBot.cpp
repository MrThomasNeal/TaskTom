#include "MyBot.h"
#include <dpp/dpp.h>

// Discord Bot Token
const std::string BOT_TOKEN = "";

int main()
{
	// Create bot cluster
	dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content);

	// Output simple log messages to stdout
	bot.on_log(dpp::utility::cout_logger());

	// Register slash commands
	bot.on_ready([&bot](const dpp::ready_t& event) {
		// Wrap command registration in run_once to make sure it doesnt run on every full reconnection 
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands {
				{ "ping", "Ping pong!", bot.me.id },
				{ "pomodoro", "Start a pomodoro timer!", bot.me.id }
			};
			bot.global_bulk_command_create(commands);
		}
	});

	bot.on_guild_member_add([&bot](const dpp::guild_member_add_t& event) {

		// Channel ID for welcome message to be displayed in
		dpp::snowflake welcome_channel_id = ;

		// Create the welcome message
		std::string welcome_message = "Welcome to the server, " + event.added.get_user()->format_username() + "! Nice to see you :)";

		// Send the welcome message to the specified channel
		bot.message_create(dpp::message(welcome_channel_id, welcome_message));

	});

	bot.on_message_create([&bot](const dpp::message_create_t& event) {

		// Simple testing to initially see if bot worked
		if (event.msg.content == "testing") {
			bot.message_create(dpp::message(event.msg.channel_id, "1, 2!"));
		}

	});

	// Pomodoro timer functionality
	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) -> dpp::task<void> {

		if (event.command.get_command_name() == "ping") {
			co_await event.co_reply("Pong!");
		}

		if (event.command.get_command_name() == "pomodoro") {

			// Duration for work and break times
			int work_duration = 1;
			int break_duration = 1;

			// Get user @ to ping them in following messages
			dpp::user caller = event.command.get_issuing_user();

			// Get the channel ID that the command was called from
			dpp::snowflake channel_id = event.command.channel_id;

			// Initial reply after command calling
			co_await event.co_reply(caller.get_mention() + " Pomodoro timer has started! Work duration is " + std::to_string(work_duration) + " minutes... with a " + std::to_string(break_duration) + " minute break!");

			// Start a timer for work_duration and manage what happens after the timer ends
			dpp::timer work_timer = bot.start_timer([&bot, channel_id, work_duration, break_duration, caller](const dpp::timer& timer) {
				bot.message_create(dpp::message(channel_id, caller.get_mention() + " Time for a " + std::to_string(break_duration) + " minute break!"));
				bot.stop_timer(timer);
			}, (work_duration*60));

			// Start a timer for break_duration and manage what happens after the timer ends
			dpp::timer break_timer = bot.start_timer([&bot, channel_id, work_duration, break_duration, caller](const dpp::timer& timer) {
				bot.message_create(dpp::message(channel_id, caller.get_mention() + " Your " + std::to_string(break_duration) + " minute break is over! Use /pomodoro again to start return to productivity :)"));
				bot.stop_timer(timer);
			}, (break_duration * 60));
		}

		co_return;
	});

	// Start the bot 
	bot.start(dpp::st_wait);

	return 0;
}
