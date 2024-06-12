#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <conio.h>

#include <windows.h>

/// Random
void Random_Init()
{
	srand(time(NULL));
}

int Random_Int(int min, int max)
{
	return min + rand() % (max - min + 1);
}

float Random_Float(float min, float max)
{
	float scale = rand() / (float)RAND_MAX;
	return min + scale * (max - min);
}

/// Utils
int GetNumCount(int num)
{
	int count = 0;

	if (num == 0)
		return 1;
	else if (num < 0)
		count++;

	num = abs(num);
	while (num > 0) {
		num /= 10;
		count++;
	}

	return count;
}

static const char* s_GameOverText =
"  ________                       \n"
" /  _____/_____    _____   ____  \n"
"/   \\  ___\\__  \\  /     \\_/ __ \\ \n"
"\\    \\_\\  \\/ __ \\|  Y Y  \\  ___/ \n"
" \\______  (____  /__|_|  /\\___  >\n"
"        \\/     \\/      \\/     \\/ \n"
"   ________                     \n"
"   \\_____  \\___  __ ___________ \n"
"    /   |   \\  \\/ // __ \\_  __ \\ \n"
"   /    |    \\   /\\  ___/|  | \\/ \n"
"   \\_______  /\\_/  \\___  >__|    \n"
"           \\/          \\/        \n";

static const char* s_Weapons[50] = {
	"Sword",
	"Axe",
	"Hammer",
	"Bow",
	"Crossbow",
	"Dagger",
	"Spear",
	"Halberd",
	"Mace",
	"Staff",
	"Sling",
	"Shuriken",
	"Katana",
	"Nunchaku",
	"Machete",
	"Garrote",
	"Rapier",
	"Saber",
	"Claymore",
	"Scimitar",
	"Estoc",
	"Glaive",
	"Trident",
	"Flail",
	"Cutlass",
	"Whip",
	"Two-handed Axe",
	"Bat",
	"Bo Staff",
	"War Fan",
	"Throwing Knife",
	"Blowgun",
	"Morning Star",
	"Tomahawk",
	"Longbow",
	"Repeating Crossbow",
	"Greatsword",
	"Bastard Sword",
	"Falchion",
	"Broadsword",
	"Poleaxe",
	"Lance",
	"Javelin",
	"Hand Cannon",
	"Musket",
	"Pike",
	"Scythe",
	"War Hammer",
	"Throwing Axe",
	"Sickle"
};

/// Data

typedef struct Statistics 
{
	int Attack;
	int Health;
} Statistics;

Statistics Add(Statistics s1, Statistics s2)
{
	s1.Attack += s2.Attack;
	s1.Health += s2.Health;

	return s1;
}

typedef enum EnemyType
{
	EnemyType_None,
	EnemyType_Ordinary,
	EnemyType_Warrior,
	EnemyType_Assasin

} EnemyType;

typedef enum AttackType
{
	AttackType_Normal,
	AttackType_Quick,
	AttackType_Power

} AttackType;

typedef struct MapData
{
	char* Data;
	int   Width;
	int   Height;

} MapData;

typedef struct Enemy
{
	EnemyType Type;

	int PositionX;
	int PositionY;

	Statistics Stats;

} Enemy;


typedef struct Level
{
	MapData Map;

	Enemy* Enemies;
	int    EnemiesCount;

	float Time;
	int   Kills;

} Level;

typedef struct DrawingData
{
	char* Data;
	int   Width;
	int   Height;

} DrawingData;

static DrawingData s_DrawingData;

typedef struct Player
{
	Statistics Stats;

	int Points;

	int PositionX;
	int PositionY;

} Player;

static Player s_Player;
static int    s_Running = 1;
static Level  s_Level;
static const char* s_LevelPath = "assets/example_level.sasmap";

static const int s_RandomEventTimeStep    = 5;
static float     s_RandomEventCurrentTime = 0.0f;

// Map
int CheckPosition(MapData* map, int x, int y)
{
	if (x >= 0 && x < map->Width && y >= 0 && y < map->Height)
		return 1;
	else
		return 0;
}

char Get(MapData* map, int x, int y)
{
	return map->Data[y * map->Width + x];
}

void Set(MapData* map, char c, int x, int y)
{
	map->Data[y * map->Width + x] = c;
}

int GetEmptySpacesCount(MapData* map)
{
	int n = 0;

	for (int y = 0; y < map->Height; y++)
	{
		for (int x = 0; x < map->Width; x++)
		{
			if (map->Data[y * map->Width + x] == ' ')
				n++;
		}
	}

	return n;
}

MapData Load(const char* path)
{
	MapData map = { NULL, 0, 0 };

	FILE* file = fopen(path, "r");
	if (!file)
	{
		printf("Could not load map: %s", path);
		return map;
	}

	char c;
	int maxWidth = 0;
	int width = 0;

	while ((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			if (width > maxWidth)
				maxWidth = width;

			width = 0;
			map.Height++;
		}
		else
			width++;
	}

	// File does not end with \n
	if (width)
	{
		if (width > maxWidth)
			maxWidth = width;

		map.Height++;
	}

	map.Width = maxWidth;

	const size_t dataCount = map.Height * map.Width + 1;
	map.Data = (char*)malloc(dataCount * sizeof(char));

	rewind(file);
	int col = 0;
	int row = 0;

	while ((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			while (col < map.Width)
			{
				map.Data[row * map.Width + col] = ' ';
				col++;
			}
			col = 0;
			row++;
		}
		else
		{
			map.Data[row * map.Width + col] = c;
			col++;
		}
	}

	fclose(file);

	// File does not end with \n
	while (col < map.Width)
	{
		map.Data[row * map.Width + col] = ' ';
		col++;
	}

	map.Data[dataCount - 1] = '\0';

	return map;
}

void FillMapWithRandomStuff(MapData* map)
{
	int count = Random_Int(0, GetEmptySpacesCount(map) / 4);
	int tries = count * 10;

	while (count >= 0 && tries)
	{
		int x = Random_Int(0, map->Width - 1);
		int y = Random_Int(0, map->Height - 1);

		if (Get(map, x, y) == ' ')
		{
			int r = Random_Int(0, 3);
			switch (r)
			{
			case 0:
				Set(map, '.', x, y);
				break;
			case 1:
				Set(map, ';', x, y);
				break;
			case 2:
				Set(map, '\'', x, y);
				break;
			case 3:
				Set(map, '?', x, y);
				break;
			}
			count--;
		}
		tries--;
	}

}

/// Drawing

inline int GetDrawingDataDataCount()
{
	// width + 1 accounts for the \n at the end of each line
	//       + 1 accounts for the \0 at the end of string
	return ((s_DrawingData.Width + 1) * s_DrawingData.Height) + 1;
}

void ClearDrawingData()
{
	int dataCount = GetDrawingDataDataCount();

	memset(s_DrawingData.Data, ' ', dataCount * sizeof(char));
	s_DrawingData.Data[dataCount - 1] = '\0';
	for (int pos = s_DrawingData.Width; pos < dataCount; pos += s_DrawingData.Width + 1)
		s_DrawingData.Data[pos] = '\n';
}

void ResizeDrawingData(int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	if (s_DrawingData.Data)
	{
		free(s_DrawingData.Data);
		s_DrawingData.Data = NULL;
	}

	s_DrawingData.Width = width;
	s_DrawingData.Height = height;
	s_DrawingData.Data = (char*)malloc(GetDrawingDataDataCount() * sizeof(char));

	ClearDrawingData();
}

void DrawChar(char c, int x, int y)
{
	if (x >= 0 && x < s_DrawingData.Width && y >= 0 && y < s_DrawingData.Height)
		s_DrawingData.Data[y * (s_DrawingData.Width + 1) + x] = c;
}

void DrawCharPointer(const char* data, int x, int y)
{
	if (x >= s_DrawingData.Width || y >= s_DrawingData.Height)
		return;

	const int bufferDataCount = ((s_DrawingData.Width + 1) * s_DrawingData.Height) + 1;;
	int dataIndex = 0;
	int xOffset = 0;
	int yOffset = 0;
	char c;

	while ((c = data[dataIndex]) != '\0')
	{
		if (c == '\n')
		{
			yOffset++;
			xOffset = 0;
			dataIndex++;
			continue;
		}

		int index = (y + yOffset) * (s_DrawingData.Width + 1) + x + xOffset;

		if (index >= bufferDataCount)
			return;

		if (s_DrawingData.Data[index] == '\n')
		{
			yOffset++;
			xOffset = 0;
			index = (y + yOffset) * (s_DrawingData.Width + 1) + x + xOffset;

			if (index >= bufferDataCount)
				return;

		}

		s_DrawingData.Data[index] = data[dataIndex];
		xOffset++;

		dataIndex++;
	}
}

void ShutdownDrawingData()
{
	if (s_DrawingData.Data)
	{
		free(s_DrawingData.Data);
		s_DrawingData.Data = NULL;
	}
}

void DrawDrawingData()
{
	printf("%s", s_DrawingData.Data);
}

void ClearConsole()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD  position = { 0, 0 };
	
	SetConsoleCursorPosition(handle, position);
}

// Random prize

void ShowPrizeTypeWeapon(int top, const char* name, int attack, int health)
{
	int height;
	if (top)
		height = 4;
	else
		height = s_DrawingData.Height / 2 + 4;

	const char* prizeStatsAttack = "Attack: ";
	const char* prizeStatsHealth = "Health: ";

	DrawCharPointer(name, s_DrawingData.Width / 2 - strlen(name) / 2, height);

	{
		char charBuffer[10];
		sprintf_s(charBuffer, sizeof(charBuffer), "%d", attack);

		DrawCharPointer(prizeStatsAttack, s_DrawingData.Width / 2 - strlen(charBuffer) / 2 - strlen(prizeStatsAttack) / 2, height + 1);
		DrawCharPointer(charBuffer, s_DrawingData.Width / 2 + strlen(prizeStatsAttack) / 2, height + 1);
	}

	{
		char charBuffer[10];
		sprintf_s(charBuffer, sizeof(charBuffer), "%d", health);

		DrawCharPointer(prizeStatsHealth, s_DrawingData.Width / 2 - strlen(charBuffer) / 2 - strlen(prizeStatsHealth) / 2, height + 2);
		DrawCharPointer(charBuffer, s_DrawingData.Width / 2 + strlen(prizeStatsHealth) / 2, height + 2);
	}

}

void PrizeToChoose()
{
	ClearDrawingData();

	for (int y = 2; y < s_DrawingData.Height; y++)
	{
		for (int x = 1; x < s_DrawingData.Width; x++)
		{
			if ((x == 1 || x == s_DrawingData.Width - 1 ||
				y == 2 || y == s_DrawingData.Height - 1 ||
				y == s_DrawingData.Height / 2 + 2 ||
				y == s_DrawingData.Height / 2) &&
				y != s_DrawingData.Height / 2 + 1)
			{
				DrawChar('X', x, y);
			}
		}
	}

	int weapon1 = Random_Int(0, 49);
	int weapon2 = Random_Int(0, 49);

	float multiplier = 1 + (float)s_Level.Time / 60.0f;

	Statistics stats1 = { (int)(Random_Float(1.0, 10.0f) * multiplier * Random_Float(0.7f, 1.3f)), (int)(Random_Float(1.0, 10.0f) * multiplier * Random_Float(0.7f, 1.3f)) };
	Statistics stats2 = { (int)(Random_Float(1.0, 10.0f) * multiplier * Random_Float(0.7f, 1.3f)), (int)(Random_Float(1.0, 10.0f) * multiplier * Random_Float(0.7f, 1.3f)) };

	ShowPrizeTypeWeapon(1, s_Weapons[weapon1], stats1.Attack, stats1.Health);
	ShowPrizeTypeWeapon(0, s_Weapons[weapon2], stats2.Attack, stats2.Health);

	char choosePrize[] = "Choose prize";
	DrawCharPointer(choosePrize, s_DrawingData.Width / 2 - strlen(choosePrize) / 2, 0);

	char pressW[] = "Press: P";
	DrawCharPointer(pressW, s_DrawingData.Width / 2 - strlen(pressW) / 2, s_DrawingData.Height / 2 - 2);

	char pressS[] = "Press: L";
	DrawCharPointer(pressS, s_DrawingData.Width / 2 - strlen(pressS) / 2, s_DrawingData.Height - 3);

	DrawDrawingData();

	int cond = 1;
	while (cond)
	{
		if (_kbhit())
		{
			char key = _getch();

			if (key == 'p')
			{
				s_Player.Stats = Add(s_Player.Stats, stats1);

				cond = 0;
			}
			else if (key == 'l')
			{
				s_Player.Stats = Add(s_Player.Stats, stats2);

				cond = 0;
			}
		}
	}

	ClearConsole();
}

// Fight

int DrawFightScreen(Enemy enemy)
{
	int playerBaseHealth = s_Player.Stats.Health;
	int enemyBaseHealth  = enemy.Stats.Health;

	while (s_Player.Stats.Health > 0 && enemy.Stats.Health > 0)
	{
		ClearDrawingData();

		// Health
		{
			// player
			{
				int healthValue = (float)s_Player.Stats.Health / (float)playerBaseHealth * 20.0f;

				for (int i = 0; i < healthValue; i++)
					DrawChar('#', i + 1, 1);

				char healthText[] = "Health: ";
				DrawCharPointer(healthText, 1, 2);

				char charBuffer[10];
				sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Stats.Health);
				DrawCharPointer(charBuffer, strlen(healthText) + 1, 2);

				char attackText[] = "Attack: ";
				DrawCharPointer(attackText, 1, 3);

				sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Stats.Attack);
				DrawCharPointer(charBuffer, strlen(attackText) + 1, 3);
			}

			// enemy
			{
				int healthValue = (float)enemy.Stats.Health / (float)enemyBaseHealth * 20.0f;

				for (int i = 0; i < healthValue; i++)
					DrawChar('#', s_DrawingData.Width - 1 - i, 1);

				char healthText[] = "Health: ";
				DrawCharPointer(healthText, s_DrawingData.Width - strlen(healthText) - GetNumCount(enemy.Stats.Health), 2);

				char charBuffer[10];
				sprintf_s(charBuffer, sizeof(charBuffer), "%d", enemy.Stats.Health);
				DrawCharPointer(charBuffer, s_DrawingData.Width - GetNumCount(enemy.Stats.Health), 2);

				char attackText[] = "Attack: ";
				DrawCharPointer(attackText, s_DrawingData.Width - strlen(attackText) - GetNumCount(enemy.Stats.Attack), 3);
				
				sprintf_s(charBuffer, sizeof(charBuffer), "%d", enemy.Stats.Attack);
				DrawCharPointer(charBuffer, s_DrawingData.Width - GetNumCount(enemy.Stats.Attack), 3);
			}

			// player attack
			{
				char pressText[] = "Press key to do";
				DrawCharPointer(pressText, s_DrawingData.Width / 2 - strlen(pressText) / 2, 5);

				char normalText[] = "N - normal attack";
				DrawCharPointer(normalText, s_DrawingData.Width / 2 - strlen(normalText) / 2, 7);

				char quickText[] = "Q - quick attack";
				DrawCharPointer(quickText, s_DrawingData.Width / 2 - strlen(quickText) / 2, 9);

				char powerText[] = "P - power attack";
				DrawCharPointer(powerText, s_DrawingData.Width / 2 - strlen(powerText) / 2, 11);

				ClearConsole();
				DrawDrawingData();

				AttackType attackType;

				int cond = 1;
				while (cond)
				{
					if (_kbhit())
					{
						char key = _getch();

						if (key == 'q')
						{
							attackType = AttackType_Quick;
							cond = 0;
						}
						else if (key == 'p')
						{
							attackType = AttackType_Power;
							cond = 0;
						}
						else if (key == 'n')
						{
							attackType = AttackType_Normal;
							cond = 0;
						}
					}
				}

				// Player performs attack
				{
					float attack = s_Player.Stats.Attack;

					switch (attackType)
					{
						case AttackType_Normal: 
						{
							attack *= Random_Float(0.9f, 1.1f);
							break;
						}
						case AttackType_Quick:
						{
							attack *= Random_Float(0.7f, 1.3f);
							break;
						}
						case AttackType_Power:
						{
							attack *= Random_Float(0.4f, 1.6f);
							break;
						}
					}

					switch (enemy.Type)
					{
						case EnemyType_Ordinary:
						{
							attack *= Random_Float(0.9f, 1.0f);
							break;
						}
						case EnemyType_Warrior:
						{
							attack *= Random_Float(0.7f, 1.0f);
							break;
						}
						case EnemyType_Assasin:
						{
							attack *= Random_Float(0.4f, 1.0f);
							break;
						}
					}

					attack *= Random_Float(Random_Float(0.5f, 0.9f), Random_Float(1.1f, 1.5f));
					attack += 1;

					enemy.Stats.Health -= (int)attack;

					if (enemy.Stats.Health <= 0)
					{
						s_Level.Kills++;
						ClearConsole();
						return 1;
					}
				}

				// Enemy performs attack
				{
					float attack = enemy.Stats.Attack;

					switch (enemy.Type)
					{
						case EnemyType_Ordinary:
						{
							attack *= Random_Float(0.9f, 1.0f);
							break;
						}
						case EnemyType_Warrior:
						{
							attack *= Random_Float(0.7f, 3.0f);
							break;
						}
						case EnemyType_Assasin:
						{
							attack *= Random_Float(0.4f, 6.0f);
							break;
						}
					}

					attack *= Random_Float(0.7f, 1.3f);
					attack += 1;


					s_Player.Stats.Health -= (int)attack;

					if (s_Player.Stats.Health <= 0)
					{
						ClearConsole();
						return 0;
					}
				}
			}
		}

		ClearConsole();
	}

	return enemy.Stats.Health <= 0;
}

// Player
void DrawEndScreen();

void PlayerOnUpdate(float dt)
{
	if (_kbhit())
	{
		char key = _getch();

		int newPositionX = s_Player.PositionX;
		int newPositionY = s_Player.PositionY;

		if (key == 'd')
			newPositionX++;
		else if (key == 'a')
			newPositionX--;

		if (key == 'w')
			newPositionY--;
		else if (key == 's')
			newPositionY++;

		if (CheckPosition(&s_Level.Map, newPositionX, newPositionY))
		{
			char c = Get(&s_Level.Map, newPositionX, newPositionY);

			switch (c)
			{
				case ' ':
				{
					s_Player.PositionX = newPositionX;
					s_Player.PositionY = newPositionY;

					break;
				}
				case '#': {

					break;
				}
				case '<': {
					s_Player.Stats.Health -= 3;

					if (s_Player.Stats.Health <= 0)
						DrawEndScreen();
					
					break;
				}
				case '>': {
					s_Player.Stats.Health -= 3;

					if (s_Player.Stats.Health <= 0)
						DrawEndScreen();
					
					break;
				}
				case '.': {
					s_Player.PositionX = newPositionX;
					s_Player.PositionY = newPositionY;
					s_Player.Points   += 1;

					Set(&s_Level.Map, ' ', newPositionX, newPositionY);

					break;
				}
				case '\'': {
					s_Player.PositionX = newPositionX;
					s_Player.PositionY = newPositionY;
					s_Player.Points   += 2;

					Set(&s_Level.Map, ' ', newPositionX, newPositionY);

					break;
				}
				case ';': {
					s_Player.PositionX = newPositionX;
					s_Player.PositionY = newPositionY;
					s_Player.Points   += 3;

					Set(&s_Level.Map, ' ', newPositionX, newPositionY);

					break;
				}
				case '?': {
					s_Player.PositionX = newPositionX;
					s_Player.PositionY = newPositionY;

					PrizeToChoose();

					Set(&s_Level.Map, ' ', newPositionX, newPositionY);

					break;
				}
			}

			for (int i = 0; i < s_Level.EnemiesCount; i++)
			{
				if (s_Player.PositionX == s_Level.Enemies[i].PositionX &&
					s_Player.PositionY == s_Level.Enemies[i].PositionY
					&& s_Level.Enemies[i].Type != EnemyType_None)
				{
					int result = DrawFightScreen(s_Level.Enemies[i]);

					if (result)
					{
						switch (s_Level.Enemies[i].Type)
						{
							case EnemyType_Ordinary:
							{
								s_Player.Points += 25;
								break;
							}
							case EnemyType_Warrior:
							{
								s_Player.Points += 35;
								break;
							}
							case EnemyType_Assasin:
							{
								s_Player.Points += 50;
								break;
							}
						}

						s_Level.Enemies[i].Type = EnemyType_None;
					}
					else
					{
						DrawEndScreen();
					}

					break;
				}
			}
		}

	}
}

void SetPlayerRandomPosition(MapData* map)
{
	int var = 1;

	while (var)
	{
		int x = Random_Int(0, map->Width - 1);
		int y = Random_Int(0, map->Height - 1);

		if (Get(map, x, y) == ' ')
		{
			s_Player.PositionX = x;
			s_Player.PositionY = y;

			var = 0;
		}
	}
}

// Enemy

Enemy CreateEnemy()
{
	Enemy enemy;

	float multiplier = 1 + (float)s_Level.Time / 60.0f;

	enemy.Stats.Attack = (int)(5.0 * multiplier * Random_Float(0.8f, 1.2f));
	enemy.Stats.Health = (int)(5.0 * multiplier * Random_Float(0.8f, 1.2f));

	enemy.Type = (EnemyType)(Random_Int(0, 3));

	switch (enemy.Type)
	{
		case EnemyType_None: return enemy;

		case EnemyType_Ordinary:
		{
			enemy.Stats.Attack += (int)(1.0 * multiplier);
			enemy.Stats.Health += (int)(1.0 * multiplier);
			break;
		}
		case EnemyType_Warrior: 
		{
			enemy.Stats.Attack += (int)(1.5 * multiplier);
			enemy.Stats.Health += (int)(0.5 * multiplier);
			break;
		}
		case EnemyType_Assasin: 
		{
			enemy.Stats.Attack += (int)(3.0 * multiplier);
			enemy.Stats.Health += (int)(1.0 * multiplier);
			break;
		}
	}

	// position
	{
		int cond = 1;
		int tries = 50;
		while (cond)
		{
			int x = Random_Int(0, s_Level.Map.Width - 1);
			int y = Random_Int(0, s_Level.Map.Height - 1);

			if (Get(&s_Level.Map, x, y) == ' ')
			{
				enemy.PositionX = x;
				enemy.PositionY = y;

				cond = 0;
			}
			tries--;
		}

		if (tries == 0)
		{
			enemy.Type = EnemyType_None;
			return enemy;
		}
	}

	return enemy;
}

// Level
void LevelOnUpdate(float dt)
{
	PlayerOnUpdate(dt);

	for (int i = 0; i < s_Level.EnemiesCount; i++)
	{
		int move = Random_Int(0, 10);
		if (move < 9)
			continue;

		int newPositionX = s_Level.Enemies[i].PositionX;
		int newPositionY = s_Level.Enemies[i].PositionY;

		int r = Random_Int(0, 3);
		switch (r)
		{
		case 0: 
			newPositionX++;
			break;
		case 1: 
			newPositionX--;
			break;
		case 2:
			newPositionY++;
			break;
		case 3:
			newPositionY--;
			break;
		}
		
		if (CheckPosition(&s_Level.Map, newPositionX, newPositionY) && Get(&s_Level.Map, newPositionX, newPositionY) == ' ')
		{
			s_Level.Enemies[i].PositionX = newPositionX;
			s_Level.Enemies[i].PositionY = newPositionY;
		}
	}
}

void LoadLevel()
{
	s_Player.Points       = 0;
	s_Player.Stats.Health = Random_Int(10, 20);
	s_Player.Stats.Attack = Random_Int(1, 3);

	s_Level.Map   = Load(s_LevelPath);
	s_Level.Time  = 0.0f;
	s_Level.Kills = 0;

	SetPlayerRandomPosition(&s_Level.Map);

	s_Level.EnemiesCount = Random_Int(50, 100);
	s_Level.Enemies      = (Enemy*)malloc(s_Level.EnemiesCount * sizeof(Enemy));
	for (int i = 0; i < s_Level.EnemiesCount; i++)
		s_Level.Enemies[i] = CreateEnemy();
	
	FillMapWithRandomStuff(&s_Level.Map);
}

void LevelShutDown()
{
	if (s_Level.Map.Data)
	{
		free(s_Level.Map.Data);
		s_Level.Map.Data = NULL;
	}

	if (s_Level.Enemies)
	{
		free(s_Level.Enemies);
		s_Level.Enemies = NULL;
	}
}

void DrawUI()
{
	// Points
	{
		char charBuffer[10];
		int  posX      = s_DrawingData.Width;
		int  charCount = 0;

		{
			charCount = GetNumCount(s_Player.Points);
			posX -= charCount;
			sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Points);
			DrawCharPointer(charBuffer, posX, 0);

			char text[] = "Points: ";
			charCount = sizeof(text) / sizeof(char) - 1;
			posX -= charCount;
			DrawCharPointer(text, posX, 0);
		}

		posX--;

		{
			int time = (int)s_Level.Time;
			charCount = GetNumCount(time);
			posX -= charCount;
			sprintf_s(charBuffer, sizeof(charBuffer), "%d", time);
			DrawCharPointer(charBuffer, posX, 0);

			char text[] = "Time: ";
			charCount = sizeof(text) / sizeof(char) - 1;
			posX -= charCount;
			DrawCharPointer(text, posX, 0);
		}
		
	}

	// Stats
	{
		char charBuffer[10];
		int  posX      = 0;
		int  charCount = 0;

		{
			char text[] = "Health: ";
			charCount = sizeof(text) / sizeof(char) - 1;
			DrawCharPointer(text, posX, 0);
			posX += charCount;

			charCount = GetNumCount(s_Player.Stats.Health);
			sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Stats.Health);
			DrawCharPointer(charBuffer, posX, 0);
			posX += charCount;
		}

		posX++;
		
		{
			char text[] = "Attack: ";
			charCount = sizeof(text) / sizeof(char) - 1;
			DrawCharPointer(text, posX, 0);
			posX += charCount;

			charCount = GetNumCount(s_Player.Stats.Attack);
			sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Stats.Attack);
			DrawCharPointer(charBuffer, posX, 0);
			posX += charCount;
		}	
	}
}

void ShutdownApp()
{
	LevelShutDown();
	ShutdownDrawingData();
}

void DrawEndScreen()
{
	ClearConsole();
	ClearDrawingData();

	DrawCharPointer(s_GameOverText, 0, 0);

	{
		char text[] = "Points: ";
		DrawCharPointer(text, 0, 14);

		char charBuffer[10];
		sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Player.Points);
		DrawCharPointer(charBuffer, strlen(text), 14);
	}

	{
		char text[] = "Kills: ";
		DrawCharPointer(text, 0, 15);

		char charBuffer[10];
		sprintf_s(charBuffer, sizeof(charBuffer), "%d", s_Level.Kills);
		DrawCharPointer(charBuffer, strlen(text), 15);
	}

	{
		char text[] = "Time: ";
		DrawCharPointer(text, 0, 16);

		char charBuffer[10];
		int time = (int)s_Level.Time;
		sprintf_s(charBuffer, sizeof(charBuffer), "%d", time);
		DrawCharPointer(charBuffer, strlen(text), 16);
	}

	DrawCharPointer("Press q to quit, c to continue", 0, 18);

	DrawDrawingData();

	int cond = 1;
	while (cond)
	{
		if (_kbhit())
		{
			char key = _getch();

			if (key == 'q')
			{
				s_Running = 0;
				cond      = 0;

				ShutdownApp();

				exit(0);
			}
			else if (key == 'c')
			{
				cond      = 0;
				s_Running = 1;

				LevelShutDown();
				LoadLevel();
				ClearConsole();
			}
		}
	}

}

void RandomEvent()
{
	int r = Random_Int(0, 3);

	switch (r)
	{
		// destroy wall
		case 0: 
		{
			int cond = 1;
			int tries = 50;
			while (cond && tries)
			{
				int x = Random_Int(0, s_Level.Map.Width - 1);
				int y = Random_Int(0, s_Level.Map.Height - 1);

				if (Get(&s_Level.Map, x, y) == '#')
				{
					Set(&s_Level.Map, ' ', x, y);
					cond = 0;
				}

				tries--;
			}
		
			break;
		}
		// Respawn enemy
		case 1: 
		{
			for (int i = 0; i < s_Level.EnemiesCount; i++)
			{
				if (s_Level.Enemies[i].Type == EnemyType_None)
				{
					s_Level.Enemies[i] = CreateEnemy();
					break;
				}
			}

			break;
		}
		// Add random stuff to map
		case 2: 
		{
			int count = Random_Int(1, 10);
			int tries = count * 10;

			while (count >= 0 && tries)
			{
				int x = Random_Int(0, s_Level.Map.Width - 1);
				int y = Random_Int(0, s_Level.Map.Height - 1);

				if (Get(&s_Level.Map, x, y) == ' ')
				{
					int r = Random_Int(0, 3);
					switch (r)
					{
					case 0:
						Set(&s_Level.Map, '.', x, y);
						break;
					case 1:
						Set(&s_Level.Map, ';', x, y);
						break;
					case 2:
						Set(&s_Level.Map, '\'', x, y);
						break;
					case 3:
						Set(&s_Level.Map, '?', x, y);
						break;
					}
					count--;
				}
				tries--;
			}

			break;
		}
		// UpgradeEnemy
		case 3: {
			s_Level.Enemies[Random_Int(0, s_Level.EnemiesCount - 1)] = CreateEnemy();
			break;
		}
	}
}

int main()
{
	const float target_frames      = 30.0f;
	const float target_delta_time  = 1.0f / target_frames;
	const int   ui_offsetY         = 3;
	const int   window_half_width  = 30;
	const int   window_half_height = 10;
	const int   window_width       = window_half_width  * 2 + 1;
	const int   window_height      = window_half_height * 2 + 1;

	Random_Init();
	ResizeDrawingData(window_width, window_height);

	LoadLevel();

	while (s_Running)
	{
		ClearConsole();

		// OnUpdates
		s_Level.Time += target_delta_time;

		// Random events
		s_RandomEventCurrentTime += target_delta_time;
		if (s_RandomEventCurrentTime >= s_RandomEventTimeStep)
		{
			s_RandomEventCurrentTime -= s_RandomEventTimeStep;
			RandomEvent();
		}

		LevelOnUpdate(target_delta_time);

		// Draw map
		for (int y = 0, pY = -s_DrawingData.Height / 2; y < s_DrawingData.Height; y++, pY++)
		{
			for (int x = 0, pX = -s_DrawingData.Width / 2; x < s_DrawingData.Width; x++, pX++)
			{
				if (CheckPosition(&s_Level.Map, s_Player.PositionX + pX, s_Player.PositionY + pY))
					DrawChar(Get(&s_Level.Map, s_Player.PositionX + pX, s_Player.PositionY + pY), x, y + ui_offsetY);
				else
					DrawChar('X', x, y + ui_offsetY);
			}
		}

		// Enemies
		for (int i = 0; i < s_Level.EnemiesCount; i++)
		{
			int posX = s_DrawingData.Width  / 2 - (s_Player.PositionX - s_Level.Enemies[i].PositionX);
			int posY = s_DrawingData.Height / 2 - (s_Player.PositionY - s_Level.Enemies[i].PositionY) + ui_offsetY;

			if (s_Level.Enemies[i].Type != EnemyType_None && posY >= ui_offsetY)
				DrawChar('E', posX, posY);
		}

		// Other
		DrawChar('P', window_half_width, window_half_height + ui_offsetY);
		DrawUI();
		
		// Render
		DrawDrawingData();
		ClearDrawingData();

		Sleep(target_delta_time * 1000.0f);
	}

	ShutdownApp();

	return 0;
}