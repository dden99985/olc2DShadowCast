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

struct sRayVertex
{
    float angle;  // Angle from source
    float x;
    float y;
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
    bool bLastToggleState = true;
    bool bTilesChanged = true;
    
    vector<sEdge> vecEdges;
    vector<sRayVertex> vecVisibilityPolygonPoints;
    
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
        
        // Add edges at the border of the screen
        float borderWidth = (float)nWorldWidth * fBlockWidth;
        float borderHeight = (float)nWorldHeight *fBlockWidth;
        vecEdges.push_back({0, 0, borderWidth, 0});                         // Top
        vecEdges.push_back({0, 0, 0, borderHeight});                        // Left
        vecEdges.push_back({0, borderWidth, borderWidth, borderHeight});    // Bottom
        vecEdges.push_back({borderWidth, 0, borderWidth, borderHeight});    // Right

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
    
    
    void CalculateVisibilityPolygon(float ox, float oy, float radius)
    {
        // Clean up existing polygon
        vecVisibilityPolygonPoints.clear();
        
        // Loop through each edge in PolyMap
        for(auto &e1 : vecEdges)
        {
            // Take the start point, then the end point
            for(int i = 0; i < 2; i++)
            {
                // Calculate the vector of the ray from source to this point
                float rdx, rdy;
                rdx = (i == 0 ? e1.sx : e1.ex) - ox;
                rdy = (i == 0 ? e1.sy : e1.ey) - oy;
                
                // Calculate the angle from source to this point
                float base_ang = atan2f(rdy, rdx);
                
                float ang = 0;
                // For each point, cast 3 rays, 1 directly at the point
                // and 1 a little bit on either side to peek around the corner
                for(int j = 0; j < 3; j++)
                {
                    if (j == 0) ang = base_ang - 0.00001f;
                    else if (j == 1) ang = base_ang;
                    else if (j == 2) ang = base_ang + 0.00001f;
                    
                    // Create ray along angle for required distance
                    rdx = radius * cosf(ang);
                    rdy = radius * sinf(ang);
                    
                    float min_t1 = INFINITY;
                    float min_px = 0;
                    float min_py = 0;
                    float min_ang = 0;
                    bool bValid = false;
                    
                    // Check for ray intersection with all edges
                    for(auto &e2 : vecEdges)
                    {
                        // Create line segment vector for the edge we're intersecting with
                        float sdx = e2.ex - e2.sx;
                        float sdy = e2.ey - e2.sy;
                        
                        // Skip rays that are co-liner with each other
                        if(fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
                        {
                            // t2 is the normalized distance from line segment start to line segment end of intersect point
                            float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx);
                            // t1 is normalized distance from source along ray to ray length of intersect point
                            float t1 = (e2.sx + sdx * t2 - ox) / rdx;
                            
                            // If intersect point exists along ray, and along line segment then intersect point is valid
                            if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
                            {
                                // Check if this intersect point is the closest one to the source.
                                // If it is, store this point and reject others
                                if (t1 < min_t1)
                                {
                                    min_t1 = t1;
                                    min_px = ox + rdx * t1;
                                    min_py = oy + rdy * t1;
                                    min_ang = atan2f(min_py - oy, min_px - ox);
                                    bValid = true;
                                }
                            }
                        }
                    }
                    
                    if(bValid)
                    {
                        // Add intersection point to visibility polygon perimeter
                        vecVisibilityPolygonPoints.push_back({min_ang, min_px, min_py});
                    }
                    
                }
            }
        }
        
        // Sort permineter points by angle from source.  This will allow us to draw a triangle fan.
        sort(vecVisibilityPolygonPoints.begin(),
             vecVisibilityPolygonPoints.end(),
             [&](const sRayVertex &t1, const sRayVertex &t2)
        {
            return t1.angle < t2.angle;
        });
        
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
        
        // Set tile map block to on or off
        if(GetMouse(0).bPressed)
        {
            // Toggle exists of cell at mouse location
            int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
            bLastToggleState = world[i].exist = !world[i].exist;
            bTilesChanged = true;
        }
        if(GetMouse(0).bHeld)
        {
            int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
            world[i].exist = bLastToggleState;
            bTilesChanged = true;
        }
           
        if(bTilesChanged)
        {
            // Take a region of "TileMap" and convert it to "PolyMap"
            ConvertTileMaptoPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);
            bTilesChanged = false;
        }
        
        if (GetMouse(1).bHeld)
        {
            CalculateVisibilityPolygon(fSourceX, fSourceY, 1000.0f);
        }
        
        int nRaysCast = (int)vecVisibilityPolygonPoints.size();
        
        // Remove duplicate points
        auto it = unique(
                         vecVisibilityPolygonPoints.begin(),
                         vecVisibilityPolygonPoints.end(),
                         [&](const sRayVertex &t1, const sRayVertex &t2)
        {
            return fabs(t1.x - t2.x) < 0.1f && fabs(t1.y - t2.y) < 0.1f;
        });
        vecVisibilityPolygonPoints.resize(distance(vecVisibilityPolygonPoints.begin(), it));
        
        int nRaysDrawn = (int)vecVisibilityPolygonPoints.size();
        
        // Drawing
        Clear(olc::BLACK);
        
        // If drawing rays, set an offscreen texture as our target buffer
        if (GetMouse(1).bHeld && vecVisibilityPolygonPoints.size() > 1)
        {
            // Draw each triangle in the fan
            for (int i = 0; i < vecVisibilityPolygonPoints.size() -1; i++)
            {
                FillTriangle(fSourceX,
                             fSourceY,
                             vecVisibilityPolygonPoints[i].x,
                             vecVisibilityPolygonPoints[i].y,
                             vecVisibilityPolygonPoints[i+1].x,
                             vecVisibilityPolygonPoints[i+1].y);
            }
            
            // Draw from end back to start
            FillTriangle(fSourceX,
                         fSourceY,
                         vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1].x,
                         vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1].y,
                         vecVisibilityPolygonPoints[0].x,
                         vecVisibilityPolygonPoints[0].y);
            
        }
        
        // Draw Blocks from TileMap
        int nBlockCount = 0;
        for (int x = 0; x < nWorldWidth; x++)
            for (int y = 0; y < nWorldHeight; y++)
            {
                if(world[y * nWorldWidth + x].exist)
                {
                    FillRect(x * fBlockWidth, y* fBlockWidth, fBlockWidth, fBlockWidth, olc::BLUE);
                    nBlockCount++;
                }
            }

        // Draw Edges from PolyMap
        for(auto &e : vecEdges)
        {
            DrawLine(e.sx, e.sy, e.ex, e.ey);
            FillCircle(e.sx, e.sy, 3, olc::RED);
            FillCircle(e.ex, e.ey, 3, olc::RED);
        }
        
        // Draw Info
        FillRect(0, 0, ScreenWidth(), fBlockWidth, olc::BLUE);
        DrawString(4, 4, "Blocks: " + to_string(nBlockCount) + " Edges: " + to_string(vecEdges.size()) + " Rays Cast: " + to_string(nRaysCast) + " Rays Drawn: " + to_string(nRaysDrawn));
        
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
