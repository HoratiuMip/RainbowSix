#define GTL_ECHO
#define GTL_ONE_SURFACE
#include "C:\\Hackerman\\Git\\GentleCore\\GentleCore.cpp"

using namespace Gtl;



inline static float master_scale = 1.0;



class Entity : public Rendable {
public:
    Entity() = default;

    Entity(
        Ref< Renderer >      render,
        std :: string_view   sprite,
        std :: string_view   clust
    )
        : _sprite{ render, sprite },
          _clust{ Clust2 :: from_file( clust ) }
    {
        _sprite.scale_at( master_scale ) >> _clust;
    }

protected:
    Sprite   _sprite   = {};
    Clust2   _clust    = {};

    bool     _kod      = false;

public:
    Clust2& operator () () {
        return _clust;
    }

public:
    void ko() {
        _kod = true;
    }

    bool is_kod() {
        return _kod;
    }

public:
    virtual void render( Ref< Renderer > render, [[ maybe_unused ]] Vec2 = {} ) const override {
        render( _sprite, _clust );
    }

    virtual void refresh( double elapsed ) {}

};



class Spir : public Entity {
public:
    using Entity :: Entity;

public:
    inline static double   speed   = 2.2;

    inline static Clock    clk     = {};

protected:
    Vec2   _dest   = {};

public:
    virtual void refresh( double elapsed ) override {
        double step = speed * elapsed;

        if( double dist = _clust().dist_to( _dest ); dist > step ) {
            _clust.origin_ref().approach( _dest, step );

            _sprite.spinZ_at( 20.0 * std :: sin( clk.up_time() * 5.0 ) );
        } else {
            _clust = _dest;

            _sprite.spinZ_at( 0.0 );
        }
    }

public:
    void dest_to( Vec2 vec ) {
        _dest = vec;
    }

};



class Gold : public Entity {
public:
    Gold() = default;

    Gold( Ref< Renderer > render )
        : Entity{ render, "Stuff\\gold.png", "" }
    {
        new ( &_brush ) Radial_brush{
            render,
            Vec2 :: O(),
            Vec2 :: O(),
            0, 0,
            std :: array< std :: pair< Chroma, float >, 2 >{
                std :: make_pair< Chroma, float >( { 0.0, 0.0, 0.0, 0.0 }, 0.0 ),
                std :: make_pair< Chroma, float >( { 0.0, 0.0, 0.0, 1.0 }, 1.0 )
            },
            Env :: W()
        };
    }

public:
    inline static double   show_time   = 1.0;
    inline static double   dark_rad    = 540.0;

private:
    inline static Ray2   _ray   = { Vec2{ -Env :: H_W(), 0 }, Vec2{ Env :: W(), 0 } };

protected:
    Radial_brush   _brush   = {};
    Clock          _clk     = {};

public:
    virtual void render( Ref< Renderer > render, [[ maybe_unused ]] Vec2 = {} ) const override {
        double x = std :: min( 1.0, _clk.up_time() / show_time );

        Vec2 spr_vec = {
            0,
            std :: pow( x * 2.0 - 1.0, 7.0 ) * Env :: H_H()
        };

        _brush.rad_to( { std :: abs( spr_vec.y ) + dark_rad } );

        render( _sprite, spr_vec )( _ray, *const_cast< Radial_brush* >( &_brush ) );

        if( x == 1.0 )
            ko();
    }

public:
    static void add( Ref< Renderer > render, auto& container ) {
        container.emplace_back( new Gold{ render } );
    }

};



class Rainbow : public Entity {
public:
    using Entity :: Entity;

public:
    inline static double    build_time   = 1.0;
    inline static int       rate         = 3;

    inline static Clust2*   col_clt      = nullptr;

protected:
    Clock   _clk   = {};

public:
    virtual void render( Ref< Renderer > render, [[ maybe_unused ]] Vec2 = {} ) const override {
        _sprite.botright_to( {
            _sprite.width() * std :: min( 1.0, _clk.up_time() / build_time ),
            _sprite.height()
        } );

        render( _sprite, _clust );
    }

    virtual void refresh( double elapsed ) override {
        if( _clust.X< bool >( *col_clt ) )
            ko();
    }

private:
    typedef   std :: pair< std :: string_view, std :: string_view >   Path_pair;

    static Path_pair _get_paths() {
        static Path_pair pairs[] = {
            {
                "Stuff\\rain_1.png", "Stuff\\rain_1.gtl.clt2"
            }
        };

        return pairs[ rand() % std :: size( pairs ) ];
    }

    Vec2 _get_vec() {
        return {
            rand() % static_cast< int >( Env :: W() - _sprite.width() ) - Env :: H_W(),
            Env :: H_H() - rand() % static_cast< int >( Env :: H() - _sprite.height() )
        };
    }

public:
    static void add( Ref< Renderer > render, auto& container ) {
        auto [ s_path, c_path ] = _get_paths();

        auto ptr = new Rainbow{ render, s_path, c_path };

        do{
            ptr -> operator()() = ptr -> _get_vec();
        } while( ptr -> operator()().X< bool >( *col_clt ) );

        container.emplace_front( ptr );
    }

};



class SFX {
public:
    SFX() = default;

    SFX( Ref< Audio > audio ) {
        _sounds.reserve( std :: size( paths ) ); /* CRITICAL */

        for( auto& path : paths )
            _sounds.emplace_back( audio, path );
    }

private:
    std :: vector< Sound >   _sounds   = {};

public:
    inline static std :: string_view paths[] = {
        "Stuff\\gold.wav",
        "Stuff\\song.wav",
        "Stuff\\boop.wav"
    };

    enum Sound_idx : size_t {
        GOLD, SONG, BOOP
    };

public:
    Ref< Sound > operator [] ( Sound_idx idx ) {
        return _sounds[ idx ];
    }

};



int main() { std :: cout << "Hello""World";
    master_scale = static_cast< float >( Env :: H() ) / 864.0;


    Surface surf{ "RainbowSix", Coord< int >{}, Size< int >{ Env :: W(), Env :: H() } };

    Renderer render{ surf };


    SFX sfx{};

    Audio audio{ Audio :: devices()[ 0 ], 44100, 1, 32, 256 };

    new ( &sfx ) SFX{ audio };


    Sprite background{ render, "Stuff\\grass.png" };

    background.height_to_keep( Env :: H() );


    std :: list< Unique< Entity > > ents = {};

    auto& spir = *static_cast< Spir* >(
        ents.emplace_back( new Spir{ render, "Stuff\\spir.png", "Stuff\\spir.gtl.clt2" } )
        .get()
    );


    Rainbow :: col_clt = &spir();


    surf.on< Surface :: KEY >( [ & ] (
        Key key, Key :: State state,
        [[ maybe_unused ]] Ref< Surface :: Trace >
    ) -> void {
        switch( key ) {
            case Key :: LMB: {
                if( state == Key :: DOWN ) {
                    Vec2 vec = Mouse :: vec();

                    if( spir().contains( vec ) )
                        sfx[ SFX :: BOOP ].play();
                    else
                        spir.dest_to( vec );
                }

                break; }

            case Key :: M: {
                if( state == Key :: UP )
                    audio.swap_mute();

                break; }
        }

    } );


    {
        std :: ifstream file{ "settings.gtl.auto_nav.txt" };

        std :: string_view sett_idf[] = {
            "spirdus_speed:", "rainbow_build_time:", "gold_show_time:",
            "gold_dark_rad:", "rainbow_spawn_rate:"
        };

        File :: auto_nav(
            file, sett_idf, sett_idf + std :: size( sett_idf ),
            [ & ] ( ptrdiff_t idx, [[ maybe_unused ]] std :: string& ) -> void {
                switch( idx ) {
                    case 0: file >> Spir :: speed; return;
                    case 1: file >> Rainbow :: build_time; return;
                    case 2: file >> Gold :: show_time; return;
                    case 3: file >> Gold :: dark_rad; return;
                    case 4: file >> Rainbow :: rate; return;
                }
            }
        );
    }


    srand( Clock :: UNIX() );


    while( !Key :: down( Key :: ESC ) ) {
        static Clock clk;
        double elapsed = clk.lap() * 144.0;


        render.begin()( background, Vec2 :: O() );


        if( int sec = static_cast< int >( clk.up_time() ); sec % Rainbow :: rate == 0 ) {
            static int last = -1;

            if( sec != std :: exchange( last, sec ) )
                Rainbow :: add( render, ents );
        }


        ents.remove_if( [ & ] ( auto& ent ) -> bool {
            ent -> refresh( elapsed );
            render( *ent );

            if( ent -> is_kod() && dynamic_cast< Rainbow* >( ent.get() ) ) {
                static size_t collected = 0;

                if( collected++ % 6 == 0 )
                    sfx[ SFX :: SONG ].play();
                
                sfx[ SFX :: GOLD ].play();

                Gold :: add( render, ents );
            }

            return ent -> is_kod();
        } );


        render.end();
    }


    return 0;
}
