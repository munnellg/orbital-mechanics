#include <SDL2/SDL.h>

#define DEFAULT_WINDOW_WIDTH     800
#define DEFAULT_WINDOW_HEIGHT    600
#define DEFAULT_WINDOW_FLAGS     0
#define DEFAULT_ECCENTRICITY     0.0
#define DEFAULT_SEMI_MAJOR_AXIS  100.0

#define FRAMES_PER_SECOND 60
#define MS_PER_FRAME      (1000 / FRAMES_PER_SECOND)

#define MAX(x,y) (((x)>(y))? (x) : (y))
#define MIN(x,y) (((x)<(y))? (x) : (y))

#define NON_ZERO(x) (((x)==0)? 0.000001f : (x))

struct
{
	SDL_Window   *window;
	SDL_Renderer *renderer;
	int window_width;
	int window_height;
	int window_flags;
	int quit;

	int focus_x; /* x of mass at centre of orbit */
	int focus_y; /* y of mass at centre of orbit */

	float x;     /* x of satellite    */
	float y;     /* y of satellite    */
	float r;     /* distance          */
	float e;     /* eccentricity      */
	float alpha; /* semi-major axis   */
	float theta; /* true anomaly      */
	float p;     /* semi-latus rectum */

	char **args;
	int numargs;
} state;

static void
clean_up(void)
{
	if (state.renderer) {
		SDL_DestroyRenderer(state.renderer);
	}

	if (state.window) {
		SDL_DestroyWindow(state.window);
	}

	SDL_Quit();
}

static void
die(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);

	clean_up();
	exit(0);
}

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s [options]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t-h\n");
	fprintf(stderr, "\t\tDisplay this help message\n");

	clean_up();
	exit(0);
}

static void
update(void)
{
	/* move the satellite */
	state.theta += 0.01;

	/* compute new distance from focus */
	state.r = state.p / NON_ZERO(1 + state.e * cos(state.theta));

	/* polar to cartesian centred on focus */
	state.x = state.focus_x + state.r * cos(state.theta);
	state.y = state.focus_y + state.r * sin(state.theta);
}

static void
handle_events(void)
{
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			state.quit = 1;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				state.quit = 1;
				break;
			}
		}
	}
}

static void
start_up(void)
{
	int err;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		die("%s", SDL_GetError());
	}

	err = SDL_CreateWindowAndRenderer(		
		state.window_width,
		state.window_height,
		state.window_flags,
		&state.window,
		&state.renderer
	);

	if (err < 0) {
		die("%s", SDL_GetError());	
	}

	state.focus_x = state.window_width >> 1;
	state.focus_y = state.window_height >> 1;

	/* semi-latus rectum is constant */
	state.p = state.alpha * (1 - state.e * state.e);

	/* clear the screen before we begin drawing the orbit */
	SDL_RenderClear(state.renderer);
}

static void
render(void)
{
	SDL_SetRenderDrawColor(state.renderer, 0x00, 0x00, 0x00, 0xFF);
	/* SDL_RenderClear(state.renderer); */
	SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderDrawPoint(state.renderer, state.focus_x, state.focus_y);
	SDL_SetRenderDrawColor(state.renderer, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderDrawPoint(state.renderer, state.x, state.y);
    SDL_RenderPresent(state.renderer);
}

static void
run(void)
{
	while (state.quit != 1) {
		Uint32 start = SDL_GetTicks();
		update();
		render();
		handle_events();
        Sint32 delay = MS_PER_FRAME - (SDL_GetTicks() - start);
        SDL_Delay(MAX(delay, 0));
	}
}

static void
parse_args(int argc, char *argv[])
{
	char *progname = argv[0];

	/* initialise the programme to default values */
	memset(&state, 0, sizeof(state));	
	state.window_width  = DEFAULT_WINDOW_WIDTH;
	state.window_height = DEFAULT_WINDOW_HEIGHT;
	state.window_flags  = DEFAULT_WINDOW_FLAGS;
	state.e             = DEFAULT_ECCENTRICITY;
	state.alpha         = DEFAULT_SEMI_MAJOR_AXIS;
	state.args          = argv;

	/* parse arguments and update programme config as appropriate */
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-h") == 0) {
				/* display help message */
				usage(progname);
			} else if (strcmp(argv[i], "-f") == 0) {
				/* enable fullscreen */
				state.window_flags |= SDL_WINDOW_FULLSCREEN;
			} else if (strcmp(argv[i], "-e") == 0) {
				/* set eccentricity from user input */
				if (++i >= argc) {
					die("-e flag expects a numerical argument");
				}

				state.e = atof(argv[i]);
			} else if (strcmp(argv[i], "-a") == 0) {
				/* set semi-major axis from user input */
				if (++i >= argc) {
					die("-a flag expects a numerical argument");
				}

				state.alpha = atof(argv[i]);
			} else {
				die("Unknown flag \"%s\"", argv[i]);
			}
		} else {
			/* add anything that is not a flag to list of arguments */
			state.args[state.numargs++] = argv[i];
		}
	}
}

int
main(int argc, char *argv[])
{
	parse_args(argc, argv);

	start_up();

	run();

	clean_up();

	return EXIT_SUCCESS;
}
