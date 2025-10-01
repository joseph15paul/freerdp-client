#include <iostream>
#include <RdpClient.hxx>
#define TAG CLIENT_TAG("custom-client")

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

#include <set>
#include <iostream>
#include <thread>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

std::set<connection_hdl, std::owner_less<connection_hdl>> clients;
std::mutex clients_mutex;
server ws_server;

void on_open(server *s, connection_hdl hdl)
{
	std::lock_guard<std::mutex> lock(clients_mutex);
	clients.insert(hdl);
	std::cout << "Client connected.\n";
}

void on_close(server *s, connection_hdl hdl)
{
	std::lock_guard<std::mutex> lock(clients_mutex);
	clients.erase(hdl);
	std::cout << "Client disconnected.\n";
}

void broadcast(server &s, const void *message, size_t len)
{
	std::lock_guard<std::mutex> lock(clients_mutex);
	for (auto &hdl : clients)
	{
		s.send(hdl, message, len, websocketpp::frame::opcode::binary);
	}
}

// UINT16 ascii_to_scancode(const char c)
// {
// 	switch (tolower(c))
// 	{
// 	case 'a':
// 		return 0x1E;
// 	case 'b':
// 		return 0x30;
// 	case 'c':
// 		return 0x2E;
// 	case 'd':
// 		return 0x20;
// 	case 'e':
// 		return 0x12;
// 	case 'f':
// 		return 0x21;
// 	case 'g':
// 		return 0x22;
// 	case 'h':
// 		return 0x23;
// 	case 'i':
// 		return 0x17;
// 	case 'j':
// 		return 0x24;
// 	case 'k':
// 		return 0x25;
// 	case 'l':
// 		return 0x26;
// 	case 'm':
// 		return 0x32;
// 	case 'n':
// 		return 0x31;
// 	case 'o':
// 		return 0x18;
// 	case 'p':
// 		return 0x19;
// 	case 'q':
// 		return 0x10;
// 	case 'r':
// 		return 0x13;
// 	case 's':
// 		return 0x1F;
// 	case 't':
// 		return 0x14;
// 	case 'u':
// 		return 0x16;
// 	case 'v':
// 		return 0x2F;
// 	case 'w':
// 		return 0x11;
// 	case 'x':
// 		return 0x2D;
// 	case 'y':
// 		return 0x15;
// 	case 'z':
// 		return 0x2C;
// 	case '0':
// 		return 0x0B;
// 	case '1':
// 		return 0x02;
// 	case '2':
// 		return 0x03;
// 	case '3':
// 		return 0x04;
// 	case '4':
// 		return 0x05;
// 	case '5':
// 		return 0x06;
// 	case '6':
// 		return 0x07;
// 	case '7':
// 		return 0x08;
// 	case '8':
// 		return 0x09;
// 	case '9':
// 		return 0x0A;
// 	case '\n':
// 		return RDP_SCANCODE_RETURN; // Enter
// 	case '\t':
// 		return RDP_SCANCODE_TAB; // TAB
// 	default:
// 		return 0; // unsupported char
// 	}
// }

// enum class MouseEventType
// {
// 	Move = 0,
// 	Down = 1,
// 	Up = 2,
// 	Wheel = 3
// };

// enum class MouseButton
// {
// 	Left = 0,
// 	Right = 1,
// 	Middle = 2,
// 	None = 3 // For movement only
// };

// enum class KeyboardEventType
// {
// 	KeyDown = 0,
// 	KeyUp = 1
// };

// struct MouseEvent
// {
// 	MouseEventType type;
// 	MouseButton button;
// 	int x;
// 	int y;
// 	int delta; // optional, for wheel
// };

// struct KeyboardEvent
// {
// 	KeyboardEventType type;
// 	int keyCode;
// };

// int surface_bits_callback(rdpContext *context, const SURFACE_BITS_COMMAND *cmd)
// {
// 	WLog_DBG(TAG, "surfacebits update called with rgba_data");
// 	auto width = cmd->bmp.width;
// 	auto height = cmd->bmp.height;
// 	auto *rgba_data = static_cast<uint8_t *>(malloc(width * height * 4));
// 	agbr_to_rgba(rgba_data, cmd->bmp.bitmapData, width, height);
// 	if (!rgba_data)
// 	{
// 		WLog_ERR(TAG, "Failed to allocate memory for rgba_data");
// 		return -1;
// 	}
// 	DeltaPatch patch;
// 	patch.top_left_x = cmd->destLeft;
// 	patch.top_left_y = cmd->destTop;
// 	patch.bottom_right_x = cmd->destRight;
// 	patch.top_left_y = cmd->destBottom;
// 	patch.rgba = rgba_data;
// 	return 0;
// }

// void handleMousePayload(rdpInput *input, const nlohmann::json &data)
// {
// 	if (!input)
// 		return;

// 	auto type = static_cast<MouseEventType>(data["eventType"].get<int>());
// 	auto button = static_cast<MouseButton>(data["button"].get<int>());
// 	int x = data["x"];
// 	int y = data["y"];

// 	UINT16 flags = 0;

// 	switch (type)
// 	{
// 	case MouseEventType::Move:
// 		flags = PTR_FLAGS_MOVE;
// 		break;

// 	case MouseEventType::Down:
// 		switch (button)
// 		{
// 		case MouseButton::Left:
// 			flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON1;
// 			break;
// 		case MouseButton::Right:
// 			flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON2;
// 			break;
// 		case MouseButton::Middle:
// 			flags = PTR_FLAGS_DOWN | PTR_FLAGS_BUTTON3;
// 			break;
// 		default:
// 			return;
// 		}
// 		break;

// 	case MouseEventType::Up:
// 		switch (button)
// 		{
// 		case MouseButton::Left:
// 			flags = PTR_FLAGS_BUTTON1;
// 			break;
// 		case MouseButton::Right:
// 			flags = PTR_FLAGS_BUTTON2;
// 			break;
// 		case MouseButton::Middle:
// 			flags = PTR_FLAGS_BUTTON3;
// 			break;
// 		default:
// 			return;
// 		}
// 		break;

// 	case MouseEventType::Wheel:
// 	{
// 		// int delta = data.value("delta", 0);
// 		// flags = PTR_FLAGS_WHEEL;
// 		// delta = std::clamp(delta, -255, 255);
// 		// freerdp_input_send_mouse_event_ex(input, flags, x, y, delta);
// 		return;
// 	}
// 	}

// 	freerdp_input_send_mouse_event(input, flags, x, y);
// }
// std::unordered_map<int, UINT16> keyCodeToScanCode = {
// 	{8, 0x0E},	// Backspace
// 	{9, 0x0F},	// Tab
// 	{13, 0x1C}, // Enter
// 	{16, 0x2A}, // Shift
// 	{17, 0x1D}, // Ctrl
// 	{18, 0x38}, // Alt
// 	{27, 0x01}, // Escape
// 	{32, 0x39}, // Space

// 	{37, 0x4B}, // Left Arrow
// 	{38, 0x48}, // Up Arrow
// 	{39, 0x4D}, // Right Arrow
// 	{40, 0x50}, // Down Arrow

// 	// Letters A–Z
// 	{65, 0x1E}, // A
// 	{66, 0x30}, // B
// 	{67, 0x2E}, // C
// 	{68, 0x20}, // D
// 	{69, 0x12}, // E
// 	{70, 0x21}, // F
// 	{71, 0x22}, // G
// 	{72, 0x23}, // H
// 	{73, 0x17}, // I
// 	{74, 0x24}, // J
// 	{75, 0x25}, // K
// 	{76, 0x26}, // L
// 	{77, 0x32}, // M
// 	{78, 0x31}, // N
// 	{79, 0x18}, // O
// 	{80, 0x19}, // P
// 	{81, 0x10}, // Q
// 	{82, 0x13}, // R
// 	{83, 0x1F}, // S
// 	{84, 0x14}, // T
// 	{85, 0x16}, // U
// 	{86, 0x2F}, // V
// 	{87, 0x11}, // W
// 	{88, 0x2D}, // X
// 	{89, 0x15}, // Y
// 	{90, 0x2C}, // Z

// 	// Digits 0–9
// 	{48, 0x0B}, // 0
// 	{49, 0x02}, // 1
// 	{50, 0x03}, // 2
// 	{51, 0x04}, // 3
// 	{52, 0x05}, // 4
// 	{53, 0x06}, // 5
// 	{54, 0x07}, // 6
// 	{55, 0x08}, // 7
// 	{56, 0x09}, // 8
// 	{57, 0x0A}, // 9
// };

// static BOOL flag = FALSE;
// #define RDP_SCANCODE_EXTENDED(_rdp_scancode) (((_rdp_scancode) & KBDEXT) ? TRUE : FALSE)

// void handleKeyboardPayload(rdpInput *input, const nlohmann::json &data)
// {
// 	KeyboardEventType type = static_cast<KeyboardEventType>(data["eventType"].get<int>());
// 	int keyCode = data["keyCode"]; // Browser keyCode (NOT scan code)

// 	// Map browser keyCode to RDP scan code (example uses 1:1)
// 	// You should create a proper lookup table for full compatibility
// 	UINT16 scancode = keyCodeToScanCode[keyCode];

// 	UINT16 flags = (RDP_SCANCODE_EXTENDED(scancode) ? KBD_FLAGS_EXTENDED : 0);
// 	char a = 'd';
// 	if (type == KeyboardEventType::KeyUp)
// 	{
// 		flags |= KBD_FLAGS_RELEASE;
// 		a = 'u';
// 	}
// 	else
// 	{
// 		a = 'd';
// 	}
// 	WLog_DBG(TAG, "sending keystroke %d : %c", keyCode, a);
// 	freerdp_input_send_keyboard_event(input, flags, scancode);
// }
#include <csignal>
#include <atomic>
std::atomic<bool> running(true);

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    running = false;  // Tell loop to stop
}

int main(int argc, char *argv[])
{
	std::signal(SIGINT, signalHandler);
	std::cout << "Press Ctrl+C to stop...\n";
	int rc = -1;
	ws_server.clear_access_channels(websocketpp::log::alevel::all);
	ws_server.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);

	ws_server.init_asio();
	ws_server.set_open_handler(std::bind(&on_open, &ws_server, std::placeholders::_1));
	ws_server.set_close_handler(std::bind(&on_close, &ws_server, std::placeholders::_1));

	ws_server.set_message_handler([&](connection_hdl hdl, server::message_ptr msg)
								  {
        std::cout << "Received: " << msg->get_payload() << std::endl;
		nlohmann::json data = nlohmann::json::parse(msg->get_payload());

    std::string type = data["type"];});
    // if (type == "mouse") {
    //     handleMousePayload(context->input, data);
    // } else if (type == "keyboard") {
    //     handleKeyboardPayload(context->input, data);
    // } });

	ws_server.listen(9002);
	ws_server.start_accept();

	// Run server in separate thread
	auto rdp = createRdpClient();
	std::thread server_thread([&]() { ws_server.run(); });
 
	rdp->Setup("127.0.0.1", "user", "root"); 
	rdp->StartAndConnect();

	std::cout << "starting read\n";
    while (running) {
		rdp->DispatchRdpEvents();
		auto disp = rdp->ReadDisplay();
		if (disp != nullptr) {
			broadcast(ws_server, disp, rdp->GetDisplayHeight() * rdp->GetDisplayWidth());
		}
    }

    std::cout << "Exiting program.\n";
    return 0;
}
