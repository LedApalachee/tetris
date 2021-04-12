#include <iostream>
#include <windows.h>
#include <conio.h>
#include <ctime>

#define ACTIVE_FIGURE_CHAR 'o'
#define FILLED_CELL_CHAR '#'
#define EMPTY_CELL_CHAR ' '

int interval; // time between frame changes in milliseconds
int score;
bool game_over;



///////////////////* FIELD */////////////////////////
// I used constant sizes of field bc if they're dynamic
// there is some shit in the end of field displaying
const int fieldsizeX = 10;
const int fieldsizeY = 20;
bool field[fieldsizeX][fieldsizeY];

void init_field()
{
	for (int x = 0; x < fieldsizeX; ++x)
		for (int y = 0; y < fieldsizeY; ++y)
			field[x][y] = false;
}



///////////////////* FIGURE *///////////////////
struct Coords
{
	int x, y;
};

struct Figure
{
	int parts_count;
	Coords *parts;
	Figure(int pc, Coords prts[]);
};

enum FigureType
{
	O, I, T, Z, S, L, J
};

Figure::Figure(int pc, Coords prts[])
{
	parts_count = pc;
	parts = prts;
}

// true - everything is ok
// false - collides with something
bool check_move_collision(Figure *fig, Coords move_vector)
{
	int x2, y2;
	for (int i = 0; i < fig->parts_count; ++i)
	{
		x2 = fig->parts[i].x + move_vector.x;
		y2 = fig->parts[i].y + move_vector.y;
		if (x2 < 0 || x2 >= fieldsizeX || y2 < 0 || y2 >= fieldsizeY) return false;
		if (field[x2][y2]) return false;
	}
	return true;
}

bool check_rotation_collision(Figure *fig, Coords rotation_centre)
{
	int x2, y2;
	for (int i = 0; i < fig->parts_count; ++i)
		if (i != fig->parts_count/2 - 1)
		{
			x2 = rotation_centre.x - fig->parts[i].y + rotation_centre.y;
			y2 = rotation_centre.y + fig->parts[i].x - rotation_centre.x;
			if (x2 < 0 || x2 >= fieldsizeX || y2 < 0 || y2 >= fieldsizeY) return false;
			if (field[x2][y2]) return false;
		}
	return true;
}

void rotate_figure(Figure *fig, Coords rotation_centre)
{
	int x2, y2;
	for (int i = 0; i < fig->parts_count; ++i)
		if (i != fig->parts_count/2 - 1)
		{
			x2 = rotation_centre.x - fig->parts[i].y + rotation_centre.y;
			y2 = rotation_centre.y + fig->parts[i].x - rotation_centre.x;
			fig->parts[i].x = x2;
			fig->parts[i].y = y2;
		}
}

void move_figure(Figure *fig, Coords move_vector)
{
	for (int i = 0; i < fig->parts_count; ++i)
	{
		fig->parts[i].x += move_vector.x;
		fig->parts[i].y += move_vector.y;
	}
}

void form_figure(Figure *fig, FigureType ft)
{
	if (fig->parts_count != 4) return;
	switch (ft)
	{
		case O: 
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{0,1};
			fig->parts[3] = Coords{1,1};
			break;
		case I:
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{2,0};
			fig->parts[3] = Coords{3,0};
			break;
		case T: 
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{2,0};
			fig->parts[3] = Coords{1,1};
			break;
		case S: 
			fig->parts[0] = Coords{2,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{0,1};
			fig->parts[3] = Coords{1,1};
			break;
		case Z: 
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{1,1};
			fig->parts[3] = Coords{2,1};
			break;
		case L: 
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{2,0};
			fig->parts[3] = Coords{0,1};
			break;
		case J: 
			fig->parts[0] = Coords{0,0};
			fig->parts[1] = Coords{1,0};
			fig->parts[2] = Coords{2,0};
			fig->parts[3] = Coords{2,1};
			break;
	}
	for (int i = 0; i < 4; ++i)
		fig->parts[i].x += fieldsizeX/2 - 2;
}



///////////////////* RENDER *//////////////////////////
const int rendersizeX = fieldsizeX + 3; // 3 is for two vertical borders and one '\n'
const int rendersizeY = fieldsizeY + 2; // 2 is for two horizontal borders
char render[rendersizeX * rendersizeY];

void init_render()
{
	// building field borders and '\n' symbols
	for (int x = 0; x < rendersizeX-1; ++x)
		render[x] = render[rendersizeX * rendersizeY - 2 - x] = '_';
	for (int y = 0; y < rendersizeY; ++y)
	{
		render[y * rendersizeX] = render[y * rendersizeX + rendersizeX-2] = '|';
		render[y * rendersizeX + rendersizeX - 1] = '\n';
	}
	render[0] = render[fieldsizeX+1] = ' ';
}

void update_render(Figure *f = nullptr)
{
	for (int x = 0; x < fieldsizeX; ++x)
		for (int y = 0; y < fieldsizeY; ++y)
			render[(y+1) * rendersizeX + (x+1)] = (field[x][y] ? FILLED_CELL_CHAR : EMPTY_CELL_CHAR);
	if (f)
		for (int i = 0; i < f->parts_count; ++i)
			if (f->parts[i].x >= 0 && f->parts[i].x < fieldsizeX && f->parts[i].y >= 0 && f->parts[i].y < fieldsizeY)
				render[(f->parts[i].y + 1) * rendersizeX + (f->parts[i].x + 1)] = ACTIVE_FIGURE_CHAR;
}



///////////////////* LOGIC *///////////////////
void input(Figure *fig)
{
	switch (getch())
	{
		case 'd':
			if (check_move_collision(fig, Coords{1,0})) move_figure(fig, Coords{1,0});
			break;
		case 'a':
			if (check_move_collision(fig, Coords{-1,0})) move_figure(fig, Coords{-1,0});
			break;
		case 's':
			if (check_move_collision(fig, Coords{0,2})) move_figure(fig, Coords{0,2});
			break;
		case 'j':
			if (check_rotation_collision(fig, fig->parts[1])) rotate_figure(fig, fig->parts[1]);
			break;
		case 'q':
			game_over = true;
			break;
	}
}

void clear()
{
	for (int y = fieldsizeY-1; y >= 0; --y)
	{
		bool full_row = true;
		bool no_filled_cells = true;
		for (int x = 0; x < fieldsizeX; ++x)
		{
			if (!field[x][y]) full_row = false;
			else no_filled_cells = false;
		}

		if (no_filled_cells) return;

		if (full_row)
		{
			for (int y2 = y; y2 >= 1; --y2)
				for (int x = 0; x < fieldsizeX; ++x)
					field[x][y2] = field[x][y2-1];
			++score;
			clear();
		}
	}
}

void logic(Figure *fig)
{
	// checking if figure lands on something
	// if it does then put all figure pieces on the field
	// and form a new figure
	for (int i = 0; i < fig->parts_count; ++i)
		if (fig->parts[i].y >= fieldsizeY-1 || field[fig->parts[i].x][fig->parts[i].y+1])
		{
			for (int i = 0; i < fig->parts_count; ++i)
				field[fig->parts[i].x][fig->parts[i].y] = FILLED_CELL_CHAR;
			srand(time(0));
			int figure_type = rand() % 7;
			form_figure(fig, (FigureType)figure_type);

			clear();

			// checking for figure moving impossiblity and consequensly losing
			for (int i = 0; i < fig->parts_count; ++i)
				if (field[fig->parts[i].x][fig->parts[i].y+1])
				{
					game_over = true;
					return;
				}

			return;
		}
	// move the figure down
	for (int i = 0; i < fig->parts_count; ++i)
		++fig->parts[i].y;
}



///////////////////* MAIN *////////////////////
int main(int argc, char **argv)
{
	interval = 75;
	score = 0;
	game_over = false;

	if (argc >= 2) interval = std::stoi(argv[1]);

	init_field();
	init_render();

	Coords coords[4];
	Figure active_fig(4, coords);

	// forming the figure
	srand(time(0));
	int figure_type = rand() % 7;
	form_figure(&active_fig, (FigureType)figure_type);

	while (!game_over)
	{
		update_render(&active_fig);
		std::cout << render;
		std::cout << "\n\nscore: " << score;
		Sleep(interval);
		system("cls");
		if (_kbhit()) input(&active_fig);
		logic(&active_fig);
	}

	update_render(&active_fig);
	std::cout << render;
	std::cout << "\n\nscore: " << score << '\n';

	return 0;
}