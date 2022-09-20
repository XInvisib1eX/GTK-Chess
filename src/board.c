#include "board.h"
#include <math.h>
#include <string.h>

RsvgHandle *BKing, *BQueen, *BRook, *BBishop, *BKnight, *BPawn,
		   *WKing, *WQueen, *WRook, *WBishop, *WKnight, *WPawn,
		   *BPQueen, *BPRook, *BPBishop, *BPKnight,
		   *WPQueen, *WPRook, *WPBishop, *WPKnight,
		   *Board;

char dragged_piece = 0;
int drag_row_start, drag_col_start;
int drag_pos_x, drag_pos_y;
int drag_status = 0;
int king_threatened_row = -1, king_threatened_col = -1;

const double border_perc=0.05;

void calc_size(GtkWidget* widget,
	gdouble *wmargin, gdouble *hmargin,
	gdouble *board_size,
	gdouble *cell_size,
	gdouble *w_offset, gdouble *h_offset
)
{
	guint width = gtk_widget_get_allocated_width(widget),
		  height = gtk_widget_get_allocated_height(widget);

	gdouble minimum=fmin(width, height);
	if (minimum == width) {
		*hmargin = (height - 0.96*width) / 2.;
		*wmargin = 0.02*width;
	} else {
		*wmargin = (width - 0.96*height) / 2.;
		*hmargin = 0.02*height;
	}
	*board_size = width - 2*(*wmargin);
	gdouble border_size = border_perc * (*board_size);
	*cell_size = (*board_size - 2*border_size)/8;
	*w_offset = *wmargin + border_size;
	*h_offset = *hmargin + border_size;
}

void load_textures(/* const char* pack */)
{
	BKing	= rsvg_handle_new_from_file("src/textures/classic/BKing.svg",	NULL);
	WKing	= rsvg_handle_new_from_file("src/textures/classic/WKing.svg", 	NULL);
	BQueen	= rsvg_handle_new_from_file("src/textures/classic/BQueen.svg", 	NULL);
	WQueen	= rsvg_handle_new_from_file("src/textures/classic/WQueen.svg", 	NULL);
	BRook	= rsvg_handle_new_from_file("src/textures/classic/BRook.svg", 	NULL);
	WRook   = rsvg_handle_new_from_file("src/textures/classic/WRook.svg", 	NULL);
	BBishop	= rsvg_handle_new_from_file("src/textures/classic/BBishop.svg",	NULL);
	WBishop	= rsvg_handle_new_from_file("src/textures/classic/WBishop.svg",	NULL);
	BKnight	= rsvg_handle_new_from_file("src/textures/classic/BKnight.svg",	NULL);
	WKnight	= rsvg_handle_new_from_file("src/textures/classic/WKnight.svg",	NULL);
	BPawn	= rsvg_handle_new_from_file("src/textures/classic/BPawn.svg",	NULL);
	WPawn	= rsvg_handle_new_from_file("src/textures/classic/WPawn.svg",	NULL);
	Board	= rsvg_handle_new_from_file("src/textures/classic/Board.svg",	NULL);
	BPQueen = rsvg_handle_new_from_file("src/textures/classic/BPQueen.svg",	NULL);
	BPRook	= rsvg_handle_new_from_file("src/textures/classic/BPRook.svg",	NULL);
	BPBishop= rsvg_handle_new_from_file("src/textures/classic/BPBishop.svg",NULL);
	BPKnight= rsvg_handle_new_from_file("src/textures/classic/BPKnight.svg",NULL);
	WPQueen	= rsvg_handle_new_from_file("src/textures/classic/WPQueen.svg",	NULL);
	WPRook	= rsvg_handle_new_from_file("src/textures/classic/WPRook.svg",	NULL);
	WPBishop= rsvg_handle_new_from_file("src/textures/classic/WPBishop.svg",NULL);
	WPKnight= rsvg_handle_new_from_file("src/textures/classic/WPKnight.svg",NULL);
}

RsvgHandle* resolve_piece(char piece)
{
	switch (piece) {
		case 'k': return BKing;
		case 'K': return WKing;
		case 'q': return BQueen;
		case 'Q': return WQueen;
		case 'r': return BRook;
		case 'R': return WRook;
		case 'b': return BBishop;
		case 'B': return WBishop;
		case 'n': return BKnight;
		case 'N': return WKnight;
		case 'p': return BPawn;
		case 'P': return WPawn;
		default: return NULL;
	}
}

RsvgHandle *resolve_promoted_piece(char piece)
{
	switch (piece) {
		case 'q': return BPQueen;
		case 'Q': return WPQueen;
		case 'r': return BPRook;
		case 'R': return WPRook;
		case 'b': return BPBishop;
		case 'B': return WPBishop;
		case 'n': return BPKnight;
		case 'N': return WPKnight;
		default: return NULL;
	}
}

gboolean draw_board(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	gdouble hmargin, wmargin, board_size, cell_size, w_offset, h_offset;

	calc_size(widget,
		&wmargin, &hmargin,
		&board_size,
		&cell_size,
		&w_offset, &h_offset
	);

	cairo_set_source_rgb(cr, 0.05, 0.2, 0.15);
	cairo_rectangle(
		cr,
		wmargin, hmargin, board_size, board_size
	);
	cairo_fill(cr);

	RsvgRectangle board_holder;
	board_holder.x = w_offset;
	board_holder.y = h_offset;
	board_holder.width = board_holder.height = 8*cell_size;
	rsvg_handle_render_document(
		Board,
		cr,
		&board_holder,
		NULL
	);
	int q_row=-1, r_row=-1, b_row=-1, n_row=-1;
	char q,r,b,n;
	int dir = state.flipped ? -1 : 1;
	int _pawn_promotion_col = pawn_promotion_col,
		_pawn_promotion_row = pawn_promotion_row;
	resolve_coord(&state, &_pawn_promotion_row, &_pawn_promotion_col);
	switch (pawn_promotion) {
		case 'P':
			q_row=_pawn_promotion_row;
			r_row=_pawn_promotion_row+dir;
			b_row=_pawn_promotion_row+2*dir;
			n_row=_pawn_promotion_row+3*dir;
			q='Q';r='R';b='B';n='N';
			break;
		case 'p':
			q_row=_pawn_promotion_row;
			r_row=_pawn_promotion_row-dir;
			b_row=_pawn_promotion_row-2*dir;
			n_row=_pawn_promotion_row-3*dir;
			q='q';r='r';b='b';n='n';
			break;
	}
	for (int row=0; row<8; row++) for (int col=0; col<8; col++) {
		RsvgHandle *current_piece = NULL;
		if (col == _pawn_promotion_col) {
			if (row == q_row)
				current_piece = resolve_promoted_piece(q);
			else if (row == r_row)
				current_piece = resolve_promoted_piece(r);
			else if (row == b_row)
				current_piece = resolve_promoted_piece(b);
			else if (row == n_row)
				current_piece = resolve_promoted_piece(n);
			else
				current_piece=resolve_piece(get_field(&state, row, col));
		} else
			current_piece=resolve_piece(get_field(&state, row, col));
		if (current_piece){
			gdouble x = col * cell_size, y = row * cell_size;
			RsvgRectangle piece_holder;
			piece_holder.x = w_offset + x;
			piece_holder.y = h_offset + y;
			piece_holder.width = piece_holder.height = cell_size;
			rsvg_handle_render_document(
				current_piece,
				cr,
				&piece_holder,
				NULL
			);
		}
	}
	if (drag_pos_x != -1){
		RsvgHandle *current_piece=resolve_piece(dragged_piece);
		if(current_piece){
			RsvgRectangle piece_holder;
			piece_holder.x = drag_pos_x - cell_size/2;
			piece_holder.y = drag_pos_y - cell_size/2;
			piece_holder.width = piece_holder.height = cell_size;
			rsvg_handle_render_document(
				current_piece,
				cr,
				&piece_holder,
				NULL
			);
		}
	}
	return FALSE;
}

void
drag_begin (
  GtkWidget* widget,
  GdkDragContext* context,
  gpointer user_data
)
{
	gdouble hmargin, wmargin, board_size, cell_size, w_offset, h_offset;

	calc_size(widget,
		&wmargin, &hmargin,
		&board_size,
		&cell_size,
		&w_offset, &h_offset
	);
	GdkWindow* window = gtk_widget_get_window(widget);
	GdkDevice* device = gdk_drag_context_get_device (context);
	int start_x, start_y;
	gdk_window_get_device_position (
		window,
		device,
		&start_x,
		&start_y,
		NULL
	);
	drag_col_start = (int)((start_x - w_offset) / cell_size);
	drag_row_start = (int)((start_y - h_offset) / cell_size);
	dragged_piece = get_field(&state, drag_row_start, drag_col_start);
	const char* piece_set = state.side_to_move ? "KQRBNP" : "kqrbnp";
	if (pawn_promotion == '-' && strchr(piece_set, dragged_piece)){
		set_field(&state, drag_row_start, drag_col_start,'-');
		gtk_drag_set_icon_pixbuf (
			context,
			empty_icon,
			0,
			0
		);
		drag_status = 1;
	} else {
		drag_status = 0;
	}
}

gboolean
drag_motion (
  GtkWidget* widget,
  GdkDragContext* context,
  gint x,
  gint y,
  guint time,
  gpointer user_data
)
{
	if (drag_status){
		drag_pos_x = x;
		drag_pos_y = y;
		gtk_widget_queue_draw(widget);
	}
	return TRUE;
}

gboolean
drag_failed (
  GtkWidget* self,
  GdkDragContext* context,
  GtkDragResult result,
  gpointer user_data
)
{
	cancel_drag(&state, dragged_piece, drag_row_start, drag_col_start);
	drag_pos_x = drag_pos_y = -1;
	gtk_widget_queue_draw(self);
	return TRUE;
}

gboolean
drag_drop (
  GtkWidget* widget,
  GdkDragContext* context,
  gint x,
  gint y,
  guint time,
  gpointer user_data
)
{
	gdouble hmargin, wmargin, board_size, cell_size, w_offset, h_offset;

	calc_size(widget,
		&wmargin, &hmargin,
		&board_size,
		&cell_size,
		&w_offset, &h_offset
	);

	int col = (int)((x - w_offset) / cell_size), row = (int)((y - h_offset) / cell_size);
	//printf("move from %d %d to %d %d\n", drag_row_start, drag_col_start, row, col);
	int from_row = drag_row_start, from_col = drag_col_start, to_row = row, to_col = col;
	resolve_coord(&state, &from_row, &from_col);
	resolve_coord(&state, &to_row, &to_col);
	if (drag_status && is_valid_move(&state, dragged_piece, from_row, from_col, to_row, to_col))
		next_move(&state, dragged_piece, from_row, from_col, to_row, to_col);
	else
		cancel_drag(&state, dragged_piece, drag_row_start, drag_col_start);

	gtk_widget_queue_draw(widget);
	drag_col_start = drag_row_start = 0;
	drag_pos_x = drag_pos_y = -1;
	drag_status = 0;
	if (is_mate(&state)){
        gtk_dialog_run(GTK_DIALOG (mate_dialog));
	}
    if (is_stalemate(&state)){
        gtk_dialog_run(GTK_DIALOG (stalemate_dialog));
	}
	return TRUE;
}

gboolean
board_clicked (
  GtkWidget* widget,
  GdkEventButton *event,
  gpointer user_data
)
{
	if (event->type == GDK_BUTTON_RELEASE && pawn_promotion != '-'){
		gdouble hmargin, wmargin, board_size, cell_size, w_offset, h_offset;

		calc_size(widget,
			&wmargin, &hmargin,
			&board_size,
			&cell_size,
			&w_offset, &h_offset
		);

		int col = (int)((event->x - w_offset) / cell_size),
			row = (int)((event->y - h_offset) / cell_size);
		resolve_coord(&state, &row, &col);
		if (col != pawn_promotion_col) return TRUE;
		switch (pawn_promotion) {
			case 'P':
				if (row < 4){
					promote_pawn(
						&state,
						pawn_promotion_row,
						pawn_promotion_col,
						resolve_promotion(row)
					);
					pawn_promotion = '-';
					pawn_promotion_row = pawn_promotion_col = -1;
					gtk_widget_queue_draw(widget);
					return TRUE;
				}
			case 'p':
				if (row > 3){
					promote_pawn(
						&state,
						pawn_promotion_row,
						pawn_promotion_col,
						resolve_promotion(row)
					);
					pawn_promotion = '-';
					pawn_promotion_row = pawn_promotion_col = -1;
					gtk_widget_queue_draw(widget);
					return TRUE;
				}
			default: return TRUE;
		}
	}
	return TRUE;
}
