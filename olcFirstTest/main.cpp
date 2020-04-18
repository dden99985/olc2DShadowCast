#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace std;

struct sEdge
{
    float sx, sy;
    float ex, ey;
};

struct sCell
{
    int edge_id[4];
    bool edge_exist[4];
    bool exist = false;
};

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

class ShadowCasting2D : public olc::PixelGameEngine {
    public:
    ShadowCasting2D()
    {
        sAppName = "ShadowCasting2D";
    }

private:
    sCell *world;
    int nWorldWidth = 40;
    int nWorldHeight = 30;
    
    vector<sEdge> vecEdges;
    
    void ConvertTileMaptoPolyMap(int sx, int sy, int w, int h, float fBlockWidth, int pitch)
    {
        // Clear "PolyMap"
        vecEdges.clear();
        
        // Clear all edges
        for (int x = 0; x < w; x++)
            for (int y = 0; y < h; y++)
                for (int j = 0; j < 4; j++)
                {
                    world[(y + sy) * pitch + (x + sx)].edge_exist[j] = false;
                    world[(y + sy) * pitch + (x + sx)].edge_id[j] = 0;
                }
        
        // Iterate through region from top left to bottom right
        for (int x = 0; x < w; x++)
            for (int y = 0; y < h; y++)
            {
                // Shortcut for locations of neighbors.  -1 if neighbor is off the grid
                int I = (y + sy) * pitch + (x + sx);                       // This cell
                int N = y > 0 ? (y + sy - 1) * pitch + (x + sx) : -1;      // North neighbor
                int S = y < h - 1 ? (y + sy + 1) * pitch + (x + sx) : -1;  // South neighbor
                int W = x > 0 ? (y + sy) * pitch + (x + sx - 1) : -1;      // West neighbor
                int E = x < w - 1 ? (y + sy) * pitch + (x + sx + 1) : -1;  // East neighbor

                // If this cell exists, check if it need edges
                if (world[I].exist)
                {
                    
                    // If this cell has no WESTERN neighbor, it needs a western edge
                    if(W == -1 || !world[W].exist)
                    {
                        // Either extend northern neighbor's, if it has one, or start a new one
                        if(N != -1 && world[N].edge_exist[WEST])
                        {
                            // Extend northern neighbor's edge down
                            vecEdges[world[N].edge_id[WEST]].ey += fBlockWidth;
                            world[I].edge_id[WEST] = world[N].edge_id[WEST];
                            world[I].edge_exist[WEST] = true;
                        }
                        else
                        {
                            // Neighbor doesn't have one so create a new edge
                            sEdge edge;
                            edge.sx = (sx + x) * fBlockWidth;
                            edge.sy = (sy + y) * fBlockWidth;
                            edge.ex = edge.sx;
                            edge.ey = edge.sy + fBlockWidth;
                            
                            // Add new edge to Polygon Pool
                            int edge_id = (int)vecEdges.size();
                            vecEdges.push_back(edge);
                            
                            // Update tile with edge info
                            world[I].edge_id[WEST] = edge_id;
                            world[I].edge_exist[WEST] = true;
                        }
                    }

                    // If this cell has no EASTERN neighbor, it needs a eastern edge
                    if(E == -1 || !world[E].exist)
                    {
                        // Either extend northern neighbor's, if it has one, or start a new one
                        if(N != -1 && world[N].edge_exist[EAST])
                        {
                            // Extend northern neighbor's edge down
                            vecEdges[world[N].edge_id[EAST]].ey += fBlockWidth;
                            world[I].edge_id[EAST] = world[N].edge_id[EAST];
                            world[I].edge_exist[EAST] = true;
                        }
                        else
                        {
                            // Neighbor doesn't have one so create a new edge
                            sEdge edge;
                            edge.sx = (sx + x + 1) * fBlockWidth;
                            edge.sy = (sy + y) * fBlockWidth;
                            edge.ex = edge.sx;
                            edge.ey = edge.sy + fBlockWidth;
                            
                            // Add new edge to Polygon Pool
                            int edge_id = (int)vecEdges.size();
                            vecEdges.push_back(edge);
                            
                            // Update tile with edge info
                            world[I].edge_id[EAST] = edge_id;
                            world[I].edge_exist[EAST] = true;
                        }
                    }

                    // If this cell has no NORTHERN neighbor, it needs a northern edge
                    if(N == -1 || !world[N].exist)
                    {
                        // Either extend western neighbor's, if it has one, or start a new one
                        if(W != -1 && world[W].edge_exist[NORTH])
                        {
                            // Extend western neighbor's edge right
                            vecEdges[world[W].edge_id[NORTH]].ex += fBlockWidth;
                            world[I].edge_id[NORTH] = world[W].edge_id[NORTH];
                            world[I].edge_exist[NORTH] = true;
                        }
                        else
                        {
                            // Neighbor doesn't have one so create a new edge
                            sEdge edge;
                            edge.sx = (sx + x) * fBlockWidth;
                            edge.sy = (sy + y) * fBlockWidth;
                            edge.ex = edge.sx + fBlockWidth;
                            edge.ey = edge.sy;
                            
                            // Add new edge to Polygon Pool
                            int edge_id = (int)vecEdges.size();
                            vecEdges.push_back(edge);
                            
                            // Update tile with edge info
                            world[I].edge_id[NORTH] = edge_id;
                            world[I].edge_exist[NORTH] = true;
                        }
                    }

                    // If this cell has no SOUTHERN neighbor, it needs a southern edge
                    if(S == -1 || !world[S].exist)
                    {
                        // Either extend western neighbor's, if it has one, or start a new one
                        if(W != -1 && world[W].edge_exist[SOUTH])
                        {
                            // Extend eastern neighbor's edge right
                            vecEdges[world[W].edge_id[SOUTH]].ex += fBlockWidth;
                            world[I].edge_id[SOUTH] = world[W].edge_id[SOUTH];
                            world[I].edge_exist[SOUTH] = true;
                        }
                        else
                        {
                            // Neighbor doesn't have one so create a new edge
                            sEdge edge;
                            edge.sx = (sx + x) * fBlockWidth;
                            edge.sy = (sy + y + 1) * fBlockWidth;
                            edge.ex = edge.sx + fBlockWidth;
                            edge.ey = edge.sy;
                            
                            // Add new edge to Polygon Pool
                            int edge_id = (int)vecEdges.size();
                            vecEdges.push_back(edge);
                            
                            // Update tile with edge info
                            world[I].edge_id[SOUTH] = edge_id;
                            world[I].edge_exist[SOUTH] = true;
                        }
                    }

                }
                
            }
    }
    
    
    
public:
    bool OnUserCreate() override
    {
        world = new sCell[nWorldWidth * nWorldHeight];
        
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        float fBlockWidth = 16.0f;
        float fSourceX = GetMouseX();
        float fSourceY = GetMouseY();
        
        bool tilesChanged = false;
        
        // Set tile map block to on or off
        if(GetMouse(0).bReleased)
        {
            // Toggle exists at mouse location
            int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
            world[i].exist = !world[i].exist;
            tilesChanged = true;
        }
           
        if(tilesChanged)
        {
            // Take a region of "TileMap" and convert it to "PolyMap"
            ConvertTileMaptoPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);
            tilesChanged = false;
        }
        
        // Drawing
        Clear(olc::BLACK);
        
        // Draw Blocks from TileMap
        for (int x = 0; x < nWorldWidth; x++)
            for (int y = 0; y < nWorldHeight; y++)
            {
                if(world[y * nWorldWidth + x].exist)
                {
                    FillRect(x * fBlockWidth, y* fBlockWidth, fBlockWidth, fBlockWidth, olc::BLUE);
                }
            }

        // Draw Edges from PolyMap
        for(auto &e : vecEdges)
        {
            DrawLine(e.sx, e.sy, e.ex, e.ey);
            FillCircle(e.sx, e.sy, 3, olc::RED);
            FillCircle(e.ex, e.ey, 3, olc::RED);
        }
        
        return true;
    }
};


int main(int argc, char const *argv[])
{
    ShadowCasting2D demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();

	return 0;
}
