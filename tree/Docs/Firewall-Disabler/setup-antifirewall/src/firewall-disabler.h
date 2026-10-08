#pragma once





//-------------------------------------------------------------------------------------------
// Internal use routines
bool	query_firewall( void );

bool	enable_firewall( void );
bool	disable_firewall( void );


//-------------------------------------------------------------------------------------------
// Exported routines
extern "C" __declspec(dllexport) void	QueryFirewall( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );


extern "C" __declspec(dllexport) void	EnableFirewall( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	DisableFirewall( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );
