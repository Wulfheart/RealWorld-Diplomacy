//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// Modifications (C) 2005 by Dirk Brüggemann
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Clarified Artistic License.
// In addition, to meet copyright restrictions you must also own at
// least one copy of any of the following board games:
// Diplomacy, Deluxe Diplomacy, Colonial Diplomacy or Machiavelli.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Clarified Artistic License for more details.
//
// You should have received a copy of the Clarified Artistic License
// along with this program; if not, write to the Open Source Initiative
// at www.opensource.org
//---------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "MapData.h"
#include "MapDisplay.h"
#include "Parse.h"
#include "Order.h"
#include "Game.h"
#include "Fails.h"
#include "Unit.h"
#include "Variants.h"
#include "OrdersDisplay.h"
#include "Strings.h"
#include "WindowUtil.h"
#include "ConvoyRoutes.h"

extern ConvoyRoutes convoyroutes;

bool addadjust(Orderlist olist, Order op, bool select);

void sotype2string(char *s, short type, bool main);
void stat2string(char *s, Order o, bool use_country);
void lstat2string(char *s, Order o, bool use_country);
void move2string(char *s, Order o, bool main, bool use_country);
void lmove2string(char *s, Order o, bool main, bool use_country);
void adjust2string(char *s, Order o, bool use_country);
void ladjust2string(char *s, Order o, bool use_country);
bool string2move(Order order, char *s, short expecting);
bool string2adjust(Order order, char *s);

void checkmove (Order op, bool ign_units);
bool checkcoast(Order op, Unit up);
void checkretreat(Order op);
void checkbuild(Order op);

void exec_moves(Orderlist orders[], UnitList units[], SupplyList supplies[], SupplyList lands[],
                                short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[]);
void exec_builds(Orderlist orders[], UnitList units[], SupplyList supplies[], 
                                short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[]);
void exec_retreats (Orderlist orders[], UnitList units[], SupplyList supplies[], SupplyList lands[],
                                short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[]);

bool MoveToRect(Order op, Rect *theRect);
bool SupportToRect(Order op, Rect *theRect);
bool ConvoyToRect(Order op, Rect *theRect);

int matchcoast (CoastType coast, short reg);

Order
neworder()
{
     Order order;

     order = (Order) new ordstruct;
     order->text = NULL;
	 order->unit = U_NONE;
     order->suborder = NULL;
     order->fails = R_NONE;
	 order->index = NULL;

     return order;
}

void deleteorder(Order order)
{
        if (order != NULL)
        {
                if (order->text != NULL)
                        delete [] order->text;
                if (order->suborder != NULL)
                        deleteorder(order->suborder);
                delete order;
        }
}

void
initolist(Orderlist list)
{
        register short i;

        for (i = 0; i < MAX_ORD; i++)
        {
                list[i].text = NULL;
                list[i].suborder = NULL;
        }
}

void
cleanorders (Orderlist list)
{
        register short i;

        if (list == NULL)
                return;

        for (i = 0; i < MAX_ORD; i++)
        {
                if (list[i].text != NULL)
                        delete [] list[i].text;
                list[i].text = NULL;
                if (list[i].suborder != NULL)
                        delete list[i].suborder;
                list[i].suborder = NULL;
        }
}


bool
appendorder(Orderlist olist, Order op)
{
     Order cp;

     cp = olist;
     while (cp-olist < MAX_ORD)
     {
                  if (!isorder(cp))
                  {
                     if (cp->text != NULL)
                                     delete [] cp->text;
                     if (cp->suborder != NULL)
                                     deleteorder(cp->suborder);
               *cp = *op;
               delete op;
               break;
                  }
                  else
               nextorder(cp);
     }
     if (cp-olist >= MAX_ORD)
                  return false;

     return true;
}

bool appendnmr(short country, UnitType unit, short sector, CoastType coast)
{
        Order op;
        char string[256];

        op = neworder();
        otext(op) = new char[2];
        strcpy(otext(op), "*");
        ocountry(op) = country;
        ounit(op) = unit;
        ostart(op) = sector;
        oscoast(op) = coast;
        otype(op) = O_NMR;
        ofails(op) = R_NONE;
        order2string(string, op, false, false);
        oindex(op) = AddOrderLine(string);
        return appendorder(orders[country], op);
}

bool haveorders()
{
        for (int c = 1; c < NUM_COUN; ++c)
        {
            Order cp;
                   for (cp = orders[c]; cp-orders[c] < MAX_ORD && isorder(cp); nextorder(cp))
                    {
                    if (ounit(cp) != U_NONE && otype(cp) != O_NMR)
                    {
                            return true;
                    }
            }
    }
         return false;
}

bool
addorder(Orderlist olist, Order op, bool select)
{
     Order cp;
     int oindex;
     char string[2048];

     if (adjustments && !dislodges)
             return addadjust(olist, op, select);

     cp = olist;
     while (cp-olist < MAX_ORD && isorder(cp))
     {
             if (ounit(cp) == ounit(op) && ostart(cp) == ostart(op))
                  {
                     oindex = cp->index;
                     if (cp->text != NULL)
                                     delete [] cp->text;
                     if (cp->suborder != NULL)
                                     deleteorder(cp->suborder);
               *cp = *op;
               delete op;
               cp->index = oindex;
               memset(string, 0, 2048);
               order2string(string, cp, false, false);
                   SetOrderLine(string, oindex);
                   if (select)
                                   SelectOrderLine(oindex);
               return true;
                  }
                  else
               nextorder(cp);
     }
         ofails(op) = R_NSU;
     appendorder(orders[0], op);
          return false;

}

Order copyorder(Order op)
{
        Order cp;

        cp = neworder();

        *cp = *op;
        if (op->suborder != NULL)
        {
                cp->suborder = copyorder(op->suborder);
        }

        if (op->text != NULL)
        {
                cp->text = new char[strlen(op->text)+1];
                strcpy(cp->text, op->text);
        }

        return cp;
}


void copyolist(Orderlist dest, Orderlist source)
{
        Order cp, op;

    cleanorders(dest);
        for (op = source, cp = dest; op-source < MAX_ORD && isorder(op); nextorder(op), nextorder(cp))
    {
                *cp = *op;
                if (op->suborder != NULL)
                {
                        cp->suborder = copyorder(op->suborder);
                }

                if (op->text != NULL)
                {
                        cp->text = new char[strlen(op->text)+1];
                        strcpy(cp->text, op->text);
                }
    }
}


Order
findorder(Orderlist olist, UnitType unit, short sector)
{
     Order cp;

     for (cp = olist; cp-olist < MAX_ORD && isorder(cp); nextorder(cp))
     {
             if ((unit == U_NONE || ounit(cp) == unit) && ostart(cp) == sector)
                          return cp;
     }
          return NULL;
}

void deleteallorder(Orderlist olist[], short sector)
{
        short country;
        Order cp, op;

        for (country = 1; country <= NUM_COUN; country++)
        {
                /* find the order */
                for (cp = olist[country]; cp-olist[country] < MAX_ORD && isorder(cp); nextorder(cp))
                {
                        if (ostart(cp) == sector)
                                break;
                }
                if (isorder(cp))
                {
                          if (cp->text != NULL)
                                     delete [] cp->text;
                          if (cp->suborder != NULL)
                                     deleteorder(cp->suborder);

                        /* copy over it */
                        for (op = cp; cp-olist[country] < MAX_ORD && isorder(cp); nextorder(op))
                        {
                                nextorder(cp);
                                *op = *cp;
                        }

                        setostart(op, 0);
                        setotext(op, NULL);
                }
        }
}

bool
addadjust(Orderlist olist, Order op, bool select)
{
        Order cp;
        int oindex;
        char string[256];

        if (!adjustments)
        {
                ofails(op) = R_BADTYPE;
                appendorder(orders[0], op);
                return false;
        }

        if (countries[ocountry(op)].builds == 0)
        {
                ofails(op) = R_BADTYPE;
                appendorder(orders[0], op);
                return false;
        }
        else if (countries[ocountry(op)].builds > 0 &&
                otype(op) != O_BUILD && otype(op) != O_WAIVE && otype(op) != O_PENDING)
        {
                ofails(op) = R_BADTYPE;
                appendorder(orders[0], op);
                return false;
        }
        else if (countries[ocountry(op)].builds < 0 &&
                otype(op) != O_REMOVE && otype(op) != O_DEF_REM)
        {
                ofails(op) = R_BADTYPE;
                appendorder(orders[0], op);
                return false;
        }

        if (otype(op) == O_PENDING)
        {
                deleteorder(op);
                return true;
        }

     cp = olist;
     while (cp-olist < MAX_ORD && isorder(cp))
     {
             if (ostart(cp) == ostart(op) || ostart(cp) == 0 || orderfails(cp))
                  {
                     oindex = cp->index;
                     if (cp->text != NULL)
                                     delete [] cp->text;
                     if (cp->suborder != NULL)
                                     deleteorder(cp->suborder);
               *cp = *op;
               delete op;
               cp->index = oindex;
               order2string(string, cp, false, false);
                   SetOrderLine(string, oindex);
                   if (select)
                                   SelectOrderLine(oindex);
               return true;
                  }
                  else
               nextorder(cp);
     }
     if (otype(op) == O_WAIVE)
     {
             deleteorder(op);
             return true;
     }
         ofails(op) = R_ADJUST;
     appendorder(orders[0], op);
          return false;
}

Order
findorderbyindex(short index)
{
        short c;
        Order op;

        for (c = 1; c <= NUM_COUN; c++)
        {
                for (op = startorder(orders[c]); isorder(op); nextorder(op))
                {
                        if (oindex(op) == index)
                                return op;
                }
        }

        return NULL;
}

void
clearorder(short index)
{
        char text[256];
        Order op;

        if ((op = findorderbyindex(index)) == NULL)
                return;

        if (op->suborder != NULL)
                deleteorder(op->suborder);
        op->suborder = NULL;
        op->finish = 0;
        op->f_coast = C_NONE;
        op->fails = R_NONE;
        switch (op->type)
        {
                case O_BUILD:
                case O_WAIVE:
                op->type = O_BUILD;
                op->start = 0;
                op->s_coast = C_NONE;
                break;

                case O_REMOVE:
                case O_DEF_REM:
                op->type = O_REMOVE;
                op->start = 0;
                op->s_coast = C_NONE;
                break;

                default:
                op->type = O_NMR;
                break;
        }
        order2string(text, op, false, false);
        SetOrderLine(text, oindex(op));

}


void
sorder2string(char *s, Order o, bool stat, bool use_country)
{
        if (o == NULL)
                return;
        if (obadsyntax(o))
        {
                strcpy(s, o->text);
                // if not already appended
                if (o->text[strlen(o->text)-1] != ')')
                {
                        strcat(s, " (*");
                        strcat(s, kResolutionStrings[o->fails]);
                        strcat(s, "*)");
                }
        }
        else if (stat)
                stat2string(s, o, use_country);
        else
        {
                switch (o->type)
                {
                case O_MOVE:
                case O_SUPP:
                case O_CONV:
                case O_HOLD:
				case O_TRAFO:
                case O_NMR:
                case O_NMR_DIS:
                        move2string(s, o, true, use_country);
                        break;

                case O_STAT:
                        stat2string(s, o, use_country);
                        break;

                default:
                        adjust2string(s, o, use_country);
                        break;
                }


        }
}


void
order2string(char *s, Order o, bool stat, bool use_country)
{
        if (o == NULL)
                return;
        if (obadsyntax(o))
        {
                strcpy(s, o->text);
                // if not already appended
                if (o->text[strlen(o->text)-1] != ')')
                {
                        strcat(s, " (*");
                        strcat(s, kResolutionStrings[o->fails]);
                        strcat(s, "*)");
                }
        }
        else if (stat)
                lstat2string(s, o, use_country);
        else
        {
                switch (o->type)
                {
                case O_MOVE:
                case O_SUPP:
                case O_CONV:
                case O_HOLD:
				case O_TRAFO:
                case O_NMR:
                case O_NMR_DIS:
                        lmove2string(s, o, true, use_country);
                        break;

                case O_STAT:
                        lstat2string(s, o, use_country);
                        break;

                default:
                        ladjust2string(s, o, use_country);
                        break;
                }


        }
}

void
order2stringCSV(char *s, Order o, bool stat, bool use_country)
{
        if (o == NULL)
                return;
        if (obadsyntax(o))
        {
                strcpy(s, o->text);
                // if not already appended
                if (o->text[strlen(o->text)-1] != ')')
                {
                        strcat(s, " (*");
                        strcat(s, kResolutionStrings[o->fails]);
                        strcat(s, "*)");
                }
        }
        else if (stat)
                lstat2string(s, o, use_country);
        else
        {
                switch (o->type)
                {
                case O_MOVE:
                case O_SUPP:
                case O_CONV:
                case O_HOLD:
				case O_TRAFO:
                case O_NMR:
                case O_NMR_DIS:
                        lmove2string(s, o, true, use_country);
                        break;

                case O_STAT:
                        lstat2string(s, o, use_country);
                        break;

                default:
                        ladjust2string(s, o, use_country);
                        break;
                }


        }
}


void
move2string(char *s, Order o, bool main, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }
        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        strcpy(s, sunit2string(o->unit));
        s += strlen(s);
        *s++ = ' ';
        strcpy(s, sreg2string(o->start));
        s += strlen(s);
        if (o->type == O_TRAFO)
		{
			strcpy(s, scoast2string(o->f_coast));
		}
		else
		{
			strcpy(s, scoast2string(o->s_coast));
		}
        s += strlen(s);
        sotype2string(s, o->type, main);
        s += strlen(s);
        if (o->type == O_SUPP || o->type == O_CONV)
        {
                *s++ = ' ';
                move2string(s, o->suborder, false, false);
                s += strlen(s);
        }
        else if (o->type == O_MOVE)
        {
			if (!gIgnoreConvoyRoutes)
			{
				ConvoyRoute* crp = convoyroutes.Select(o->start);
				if (crp != NULL)
				{
					Waypoint* w = crp->GetVia();
					*s++ = ' ';
					strcpy(s, sreg2string(w->GetPosition()));
					s += strlen(s);
					sotype2string(s, o->type, main);
					s += strlen(s);
					while (w->GetNext() != 0)
					{
						w = w->GetNext();
						*s++ = ' ';
						strcpy(s, sreg2string(w->GetPosition()));
						s += strlen(s);
						sotype2string(s, o->type, main);
						s += strlen(s);
					}
				}
			}
            *s++ = ' ';
            strcpy(s, sreg2string(o->finish));
            s += strlen(s);
            strcpy(s, scoast2string(o->f_coast));
            s += strlen(s);
        }

        if (main)
        {
/*                *s++ = '.'; */
                if (orderfails(o))
                {
                        strcpy(s, " (*");
                        s += 3;
                        strcat(s, kResolutionStrings[o->fails]);
                        s += strlen(s);
                        strcpy(s, "*)");
                        s +=2 ;
                }
        }
        *s = '\0';

}

void
lmove2string(char *s, Order o, bool main, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }
        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        strcpy(s, sunit2string(o->unit));
        s += strlen(s);
        *s++ = ' ';
        strcpy(s, lreg2string(o->start));
        s += strlen(s);
		if (o->type == O_TRAFO)
		{
			strcpy(s, scoast2string(o->f_coast));
		}
		else
		{
			strcpy(s, scoast2string(o->s_coast));
		}
        s += strlen(s);
        sotype2string(s, o->type, main);
        s += strlen(s);
        if (o->type == O_SUPP || o->type == O_CONV)
        {
                *s++ = ' ';
                lmove2string(s, o->suborder, false, false);
                s += strlen(s);
        }
        else if (o->type == O_MOVE)
        {
			if (!gIgnoreConvoyRoutes)
			{
				ConvoyRoute* crp = convoyroutes.Select(o->start);
				if (crp != NULL && main)
				{
					Waypoint* w = crp->GetVia();
					*s++ = ' ';
					strcpy(s, lreg2string(w->GetPosition()));
					s += strlen(s);
					sotype2string(s, o->type, main);
					s += strlen(s);
					while (w->GetNext() != 0)
					{
						w = w->GetNext();
						*s++ = ' ';
						strcpy(s, lreg2string(w->GetPosition()));
						s += strlen(s);
						sotype2string(s, o->type, main);
						s += strlen(s);
					}
				}
			}
			*s++ = ' ';
            strcpy(s, lreg2string(o->finish));
            s += strlen(s);
            strcpy(s, scoast2string(o->f_coast));
            s += strlen(s);
        }

        if (main)
        {
/*                *s++ = '.'; */
                if (orderfails(o))
                {
                        strcpy(s, " (*");
                        s += 3;
                        strcat(s, kResolutionStrings[o->fails]);
                        s += strlen(s);
                        strcpy(s, "*)");
                        s +=2 ;
                }
        }
        *s = '\0';

}

void
adjust2string(char *s, Order o, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }

        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        if (o->type == O_PENDING)
        {
                sprintf(s, "%d ", o->start);
                s += strlen(s);
        }
        sotype2string(s, o->type, true);
        s += strlen(s);
        if (o->start != 0 || o->type == O_WAIVE)
        {
                if (o->type == O_BUILD || o->type == O_REMOVE || o->type == O_DEF_REM
                        || o->type == O_DISBAND)
                {
                        *s++ = ' ';
                        strcpy(s, sunit2string(o->unit));
                        s += strlen(s);
                        *s++ = ' ';
                        strcpy(s, sreg2string(o->start));
                        s += strlen(s);
                        strcpy(s, scoast2string(o->s_coast));
                        s += strlen(s);
                }

/*                *s++ = '.'; */

                if (orderfails(o))
                {
                        strcpy(s, " (*");
                        s += 3;
                        strcat(s, kResolutionStrings[o->fails]);
                        s += strlen(s);
                        strcpy(s, "*)");
                        s +=2 ;
                }
        }
        *s = '\0';

}


void
ladjust2string(char *s, Order o, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }

        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        if (o->type == O_PENDING)
        {
                sprintf(s, "%d ", o->start);
                s += strlen(s);
        }
        sotype2string(s, o->type, true);
        s += strlen(s);
        if (o->start != 0 || o->type == O_WAIVE)
        {
                if (o->type == O_BUILD || o->type == O_REMOVE || o->type == O_DEF_REM
                        || o->type == O_DISBAND)
                {
                        *s++ = ' ';
                        strcpy(s, sunit2string(o->unit));
                        s += strlen(s);
                        *s++ = ' ';
                        strcpy(s, lreg2string(o->start));
                        s += strlen(s);
                        strcpy(s, scoast2string(o->s_coast));
                        s += strlen(s);
                }

/*                *s++ = '.'; */

                if (orderfails(o))
                {
                        strcpy(s, " (*");
                        s += 3;
                        strcat(s, kResolutionStrings[o->fails]);
                        s += strlen(s);
                        strcpy(s, "*)");
                        s +=2 ;
                }
        }
        *s = '\0';

}


void
stat2string(char *s, Order o, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }
        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        strcpy(s, sunit2string(o->unit));
        s += strlen(s);
        *s++ = ' ';
        strcpy(s, sreg2string(o->start));
        s += strlen(s);
        strcpy(s, scoast2string(o->s_coast));
        s += strlen(s);
/*        *s++ = '.'; */
        if (orderfails(o))
        {
                strcpy(s, " (*");
                s += 3;
                strcat(s, kResolutionStrings[o->fails]);
                s += strlen(s);
                strcpy(s, "*)");
                s +=2 ;
        }
        *s = '\0';

}

void
lstat2string(char *s, Order o, bool use_country)
{
        if (o == NULL)
        {
                *s = '\0';
                return;
        }
        if (use_country)
        {
                strcpy(s, country2string(o->country));
                s += strlen(s);
                *s++ = ':';
                *s++ = ' ';
        }

        strcpy(s, sunit2string(o->unit));
        s += strlen(s);
        *s++ = ' ';
        strcpy(s, lreg2string(o->start));
        s += strlen(s);
        strcpy(s, scoast2string(o->s_coast));
        s += strlen(s);
/*        *s++ = '.'; */
        if (orderfails(o))
        {
                strcpy(s, " (*");
                s += 3;
                strcat(s, kResolutionStrings[o->fails]);
                s += strlen(s);
                strcpy(s, "*)");
                s +=2 ;
        }
        *s = '\0';

}


bool
string2order(Order order, char *s)
{
        char *word;
        char string[256];
        short country=0, i;

        if (order == NULL)
                return false;
        if (s == NULL)
        {
                order->fails = R_SYNTAX;
                return false;
        }

        order->text = new char[strlen(s)+1];
        strcpy(order->text, s);
        order->suborder = NULL;

        (void) striptext(s);

        order->fails = R_NONE;
        /* figure out if we automagically fail */
        if ((word = strchr(s, '*')) != NULL)
        {
                if (*(--word) == '(' && strlen(word) > 2)
                {
                        word += 2;
                        for (i = 1; i <= R_MAX; i++)
                        {
                                if (samestring(word, kResolutionStrings[i]))
                                        order->fails = i;
                        }
                        if (order->fails == R_NONE)
                                order->fails = R_FAILS;
                }
                else
                        order->fails = R_NONE;
        }

        word = skipspace(s);

        word = string2country(&country, word);
        if (country == 0)
        {
                order->fails = R_BADCOUN;
                order->country = 0;
                return false;
        }
        order->country = country;
        if (wordlen(word) == 0)
        {
                order->fails = R_SYNTAX;
                return false;
        }

        if (samestring(word, "Build") || samestring(word, "Remove") ||
                samestring(word, "Defaults") || isdigit(*word)
                || samestring(word, "Waive")
                || samestring(word, "B ") || samestring(word, "R ")
                || samestring(word, "Disband") || samestring(word, "D "))
                return string2adjust(order, word);
        else
        {
                for (i = O_BUILD; i <= O_DISBAND; i++)
                {
                        GetIndCString(string, kMoveTypeStringsID, i);
                        if (samestring(word, string))
                        {
                                return string2adjust(order, word);
                                break;
                        }
                }
                return string2move(order, word, X_ANY);
        }
}

bool
string2move(Order order, char *word, short expecting)
{
        short type;
		ConvoyRoute* crp = NULL;

		if (expecting != X_ANY)
                word = string2country(&order->country, word);

        word = string2unit(&order->unit, word);
        if (order->unit == 0 && !gAddUnitToOrder)
        {
                /* check to see if this is really a bad country */
                word = string2unit(&order->unit, skipword(word));
                if (order->unit == 0)
					    order->fails = R_BADUNIT;
				else
                        order->fails = R_BADCOUN;
                return false;
        }

        word = string2sector(&order->start, word);
        if (order->start == 0)
        {
                order->fails = R_BADREGN;
                return false;
        }

		if (expecting == X_ANY)
		{
			convoyroutes.Remove(order->start);
		}

		if (order->unit == 0 && gAddUnitToOrder)
		{
			setounit(order, utype(sectunit(getsector(map, ostart(order)))));
		}

        word = string2coast(&order->s_coast, word);

        /* get the move type */
        word = string2otype(&order->type, word);

        switch (order->type)
        {
        case O_MOVE:
                word = string2sector(&order->finish, word);
                if (order->finish == 0)
                {
                        order->fails = R_BADREGN;
                        return false;
                }
                word = string2coast(&order->f_coast, word);

                /* take care of convoys */
                type = O_UNKN;
                word = string2otype(&type, word);
				if (type == O_MOVE && expecting == X_ANY)
				{
					crp = new ConvoyRoute(order->start);			
				}
                while (wordlen(word) > 0 && type == O_MOVE)
                {
						if (crp != NULL)
						{
							crp->AddWaypoint(order->finish);
						}
                        word = string2sector(&order->finish, word);
                        if (order->finish == 0)
                        {
                                order->fails = R_BADREGN;
								if (crp != NULL)
								{
									delete crp;
									crp = NULL;
								}
                                return false;
                        }
						word = string2coast(&order->f_coast, word);
						type = O_UNKN;
                        word = string2otype(&type, word);
                }
                if (type != O_UNKN && type != O_MOVE)
                {
                        order->fails = R_BADTYPE;
						if (crp != NULL)
						{
							delete crp;
							crp = NULL;
						}
                        return false;
                }
				if (crp != NULL)
				{
					crp->SetFinish(order->finish);
					convoyroutes.Insert(crp);
					crp = NULL;
				}
                break;

        case O_SUPP:
                order->finish = 0;
                order->f_coast = C_NONE;
                if (expecting != X_ANY)
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                if ((order->suborder = neworder()) == NULL)
                {
                        order->fails = R_SYNTAX;
                        return false;
                }
                if (!string2move(order->suborder, word, X_HORM))
                {
                        order->fails = order->suborder->fails;
                        return false;
                }
                order->finish = order->suborder->start;
                order->f_coast = order->suborder->s_coast;
				if (iscanalcenter(getsector(map, ostart(order))))
				{
					// try to get hold of the SC-part of the order
					// not sure how to do this, though
				}
                break;

        case O_CONV:
                order->finish = 0;
                order->f_coast = C_NONE;
                if (expecting != X_ANY)
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                if ((order->suborder = neworder()) == NULL)
                {
                        order->fails = R_SYNTAX;
                        return false;
                }
                if (!string2move(order->suborder, word, X_MOVE))
                {
                        order->fails = order->suborder->fails;
                        return false;
                }
                order->finish = order->suborder->start;
                order->f_coast = order->suborder->s_coast;
                break;

        case O_HOLD:
                order->finish = 0;
                order->f_coast = C_NONE;
                if (expecting != X_ANY && expecting != X_HORM)
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                break;

		case O_TRAFO:
				order->finish = order->start;
				order->f_coast = order->s_coast;
				order->s_coast = C_NONE;
				break;

        case O_DISBAND:
                order->finish = 0;
                order->f_coast = C_NONE;
                break;

        default:
                if (wordlen(word) == 0 || *word == '.' || *word == '(')
                {
                /* same as hold */
                        if (expecting == X_ANY)
                                order->type = O_NMR;
                        else
                                order->type = O_HOLD;
                        order->finish = 0;
                        order->f_coast = C_NONE;
                        if (expecting != X_ANY && expecting != X_HORM)
                        {
                                order->fails = R_BADTYPE;
                                return false;
                        }
                }
                else if (samestring(word, "can retreat to "))
                {
                        order->type = O_STAT;
                        order->fails = R_RETREAT;
                        return false;
                }
                else
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                break;

        }

        return true;
}


bool
string2adjust(Order order, char *s)
{
        char *word;
        char string[256];
        short i;

        if (s == NULL)
                return false;

        order->unit = U_NONE;

        /* get order type */
        if (isdigit(*s))
        {
                order->start = atoi(s);
                 order->type = O_PENDING;
                word = skipword(s);
                GetIndCString(string, kMoveTypeStringsID, O_PENDING);
                if (samestring(word, string))
                        return true;
                if (samestring(word, "unusable"))
                        word = skipword(word);
                if (!samestring(word, "build"))
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                word = skipword(word);
                if (!samestring(word, "pending"))
                {
                        order->fails = R_BADTYPE;
                        return false;
                }
                 return true;
        }
        else if (samestring(s, "Build") || samestring(s, "B "))
        {
                order->type = O_BUILD;
                s = skipword(s);
                if (samestring(s, "waived"))
                {
                        order->type = O_WAIVE;
                        order->start = 0;
                        return true;
                }
        }
        else if (samestring(s, "Waive"))
        {
                order->type = O_WAIVE;
                order->start = 0;
                return true;
        }
        else if (samestring(s, "Remove") || samestring(s, "R ")
                         || samestring(s, "Disband") || samestring(s, "D "))
        {
                order->type = O_REMOVE;
                s = skipword(s);
        }
        else if (samestring(s, "Defaults"))
        {
                order->type = O_DEF_REM;
                s = skipword(skipword(s));
        }

        /* cover those we missed, in case they're in some furrin tongue */
        for (i = O_BUILD; i <= O_DISBAND; i++)
        {
                GetIndCString(string, kMoveTypeStringsID, i);
                if (samestring(s, string))
                        break;
        }
        if (i <= O_DISBAND)
        {
                switch (i)
                {
                case O_BUILD:
                        order->type = O_BUILD;
                        s = skipword(s);
                        break;

                case O_WAIVE:
                        order->type = O_WAIVE;
                        order->start = 0;
                        return true;
                        break;

                case O_DISBAND:
                case O_REMOVE:
                        order->type = O_REMOVE;
                        s = skipword(s);
                        break;

                case O_DEF_REM:
                        order->type = O_DEF_REM;
                        s = skipword(skipword(s));
                        break;
                }
        }

        /* get unit type */
        if (wordlen(s) == 0)
        {
                order->unit = U_NONE;
                order->start = 0;
                return true;
        }
        if (samestring(s, "a") || samestring(s, "the "))
        {
                word = skipword(s);
                word = string2unit(&order->unit, word);
                if (order->unit == 0)
                {
                        word = string2unit(&order->unit, s);
                        if (order->unit == 0)
                        {
                                order->fails = R_BADUNIT;
                                return false;
                        }
                }
        }
        else
        {
                word = string2unit(&order->unit, s);
                if (order->unit == 0)
                {
                        order->fails = R_BADUNIT;
                        return false;
                }
        }

        /* get the sector */
        if (wordlen(word) == 0)
        {
                order->fails = R_BADREGN;
                return false;
        }
        if (samestring(word, "in "))
                word = skipword(word);
        if (wordlen(word) == 0)
        {
                order->fails = R_BADREGN;
                return false;
        }
        if (samestring(word, "the "))
                word = skipword(word);
        if (wordlen(word) == 0)
        {
                order->fails = R_BADREGN;
                return false;
        }
        word = string2sector(&order->start, word);
        if (order->start == 0)
        {
                order->fails = R_BADREGN;
                return false;
        }
        word = string2coast(&order->s_coast, word);

        return true;
}


char *
string2otype(short *type, char *s)
{
        register short i;
        register char *next_word, *match = NULL;
        char string[256];

        next_word = s;
        *type = O_UNKN;

        if ((match = samestring(s, "->")) != NULL
                || (match = samestring(s, "-")) != NULL
                || (match = samestring(s, "MOVE ")) != NULL
                || (match = samestring(s, "M ")) != NULL
                || (match = samestring(s, "to ")) != NULL
                || (match = samestring(s, "moves ")) != NULL
                || (match = samestring(s, "moves to ")) != NULL
                || (match = samestring(s, "attacks ")) != NULL
				|| (match = samestring(s, "nach ")) != NULL
				|| (match = samestring(s, "zieht nach ")) != NULL
                )
        {
                *type = O_MOVE;
                next_word = match;
        }
        else if ((match = samestring(s, "SUPPORT ")) != NULL
                || (match = samestring(s, "S ")) != NULL
                || (match = samestring(s, "Supports ")) != NULL)
        {
                *type = O_SUPP;
                next_word = match;
        }
        else if ((match = samestring(s, "CONVOY ")) != NULL
                || (match = samestring(s, "Convoys ")) != NULL
                || (match = samestring(s, "C ")) != NULL)
        {
                *type = O_CONV;
                next_word = match;
        }
        else if ((match = samestring(s, "HOLD")) != NULL
                || (match = samestring(s, "Holds")) != NULL
                || (match = samestring(s, "H")) != NULL
				|| (match = samestring(s, "XXX")) != NULL)
        {
                *type = O_HOLD;
                next_word = match;
        }
        else if ((match = samestring(s, "NO ORDER PROCESSED (DISBAND)")) != NULL)
        {
                *type = O_NMR_DIS;
                next_word = match;
        }
        else if ((match = samestring(s, "NO ORDER PROCESSED")) != NULL
                || (match = samestring(s, "NMR")) != NULL
                || (match = samestring(s, "NO MOVE RECEIVED")) != NULL)
        {
                *type = O_NMR;
                next_word = match;
        }
        else if ((match = samestring(s, "DISBAND")) != NULL)
        {
                *type = O_DISBAND;
                next_word = match;
        }
		else if ((match = samestring(s, "TRAFO")) != NULL
				|| (match = samestring(s, "Transforms")) != NULL
				|| (match = samestring(s, "T")) != NULL)
		{
				*type = O_TRAFO;
                next_word = match;
		}

        if (match != NULL)
                return skipspace(next_word);

        /* catch any we missed */
        for (i = 1; i <= O_MAX; i++)
        {
                GetIndCString(string, kMoveTypeStringsID, i);
                if ((match = samestring(s, string)) != NULL)
                {
                        *type = i;
                        next_word = match;
                        break;
                }
        }
        return skipspace(next_word);
}

void
sotype2string(char *s, short type, bool main)
{
        if ((type == O_HOLD && !main) || type <= 0)
                return;

        GetIndCString(s, kMoveTypeStringsID, type);
}


/* returns true if a move goes to this sector */
int
movesto (Orderlist olist, short sector)
{
     Order op;

     for (op = startorder(olist); isorder(op); op++)
          if (!orderfails(op) && otype(op) == O_MOVE && ofinish(op) == sector)
              return 1;
     return 0;
}

void
checkorder(Order op, bool ign_units)
{
        if (dislodges)
        {
                if (otype(op) != O_DISBAND && otype(op) != O_MOVE && otype(op) != O_NMR_DIS)
                {
                        setofails(op, R_BADTYPE);
                        return;
                }
        }
        else if (adjustments)
        {
                if (otype(op) != O_BUILD && otype(op) != O_REMOVE && otype(op) != O_PENDING
                         && otype(op) != O_WAIVE && otype(op) != O_DEF_REM)
                {
                        setofails(op, R_BADTYPE);
                        return;
                }
        }
		else if (otype(op) == O_TRAFO)
		{
				if (!gAllowsTrafo)
				{
				        setofails(op, R_BADTYPE);
                        return;
				}
		}
        else if (otype(op) != O_MOVE && otype(op) != O_SUPP && otype(op) != O_HOLD
                         && otype(op) != O_CONV && otype(op) != O_NMR && otype(op) != O_STAT)
        {
                setofails(op, R_BADTYPE);
                return;
        }
		
        switch (otype(op))
        {
        case O_MOVE:
                if (!dislodges)
                        checkmove(op, ign_units);
                else
                        checkretreat(op);
                break;

        case O_SUPP:
        case O_HOLD:
        case O_CONV:
		case O_TRAFO:
                checkmove(op, ign_units);
                break;

        case O_BUILD:
        case O_REMOVE:
        case O_DEF_REM:
                checkbuild(op);
                break;

        case O_DISBAND:
        case O_NMR_DIS:
                checkretreat(op);

        default:
                break;
        }
}


/* do some sanity checks on the move */
void
checkmove (Order op, bool ign_units)
{
     Unit up;

     if (!isorder(op))
          return;

     if (obadsyntax(op))
          return;

     /* is there a unit of the appropriate type & country there? */
     /* don't want to do this in some cases */
     if (!ign_units) 
	 {
          if ((up = getunit(units[ocountry(op)], ounit(op), ostart(op), oscoast(op))) == NULL) 
		  {
               setofails(op, R_NSU);
               return;
          }

          /* set the unit */
          if (ounit(op) == U_NON)
               setounit(op, utype(up));

          /* are the coasts right? */
          (void) checkcoast(op, up);
     }

     /* now do this for suborder */
     if (!orderfails(op) && isorder(osuborder(op)))
     {
          checkmove(osuborder(op), ign_units);
                     if (orderfails(osuborder(op)))
                               setofails(op, ofails(osuborder(op)));
            }

}


/* make sure the coasts are okay */
bool
checkcoast (Order op, Unit up)
{
     Neighbor np = NULL;

     if (!isorder(op))
          return false;

     if (obadsyntax(op))
          return false;

	 /* check starting sector */
	 if (oscoast(op) == C_NONE)
		 setoscoast(op, ucoast(up));
	 else if (oscoast(op) != ucoast(up))
		 setofails(op, R_NSU);
	 
	 /* we'll catch these on the next go-around */
     if (otype(op) == O_SUPP || otype(op) == O_CONV)
           return true;

     /* check finishing sector */
     /* check army or wing */
     if ((ounit(op) == U_ARM) || (ounit(op) == U_WNG))
     {
/*             if (ofinish(op) != 0 && map[ofinish(op)].unit != NULL)
                     ofcoast(op) = ucoast(map[ofinish(op)].unit);
             else */
		 if (otype(op) == O_TRAFO)
		 {
			 if (ounit(op) == U_WNG)
				  setofails(op, R_BADUNIT);
			 else if (ofinish(op) == 0)
				  setofails(op, R_BADREGN);
			 else if (!issea(getsector(map, ofinish(op))) || !issupp(getsector(map, ofinish(op))))                           
				  setofails(op, R_BADREGN);
             else if (ofcoast(op) != C_NONE && !isbicoastal(getsector(map, ofinish(op))))
                  setofails(op, R_BADREGN);
             /* look for given finishing coast */
             else if (isbicoastal(getsector(map, ofinish(op))))
             {
				if (ofcoast(op) == C_NONE)
				{
					setofails(op, R_BADCOAST);
				}
				else if (!(ofcoast(op) == scoast1(getsector(map, ofinish(op)))
							|| ofcoast(op) == scoast2(getsector(map, ofinish(op)))
							|| ofcoast(op) == scoast3(getsector(map, ofinish(op))))
						)
				{
					setofails(op, R_BADCOAST);
				}
			 }	
		}
		 else
		 {
			ofcoast(op) = C_NONE;
			if (ofinish(op) != 0
					&& iswingonly(getsector(map, ofinish(op))) 
					&& ounit(op) == U_ARM
					&& otype(op) != O_HOLD)                         
				setofails(op, R_BADREGN);					                           
		 }
	 }
     /* check fleet */
     else 
	 {
		 if (otype(op) == O_TRAFO)
		 {
			 if ((ofinish(op) == 0) || !issupp(getsector(map, ofinish(op))))                           
						setofails(op, R_BADREGN);
			 ofcoast(op) = C_NONE;
		 }
		 else if (ofinish(op) != 0)
		 {
			 if (!issea(getsector(map, ofinish(op))))                           
						setofails(op, R_BADREGN);
             if (ofcoast(op) != C_NONE && !isbicoastal(getsector(map, ofinish(op))))
                  setofails(op, R_BADREGN);
             /* look for neighbors with given finishing coast */
             else if (isbicoastal(getsector(map, ofinish(op))))
             {
				if (ofcoast(op) == C_NONE)
				{
					setofails(op, R_BADCOAST);
				}
				else if (ucoast(up) == C_NONE)
				{
					np = getneighbor(seaneigh(getsector(map, ostart(op))), ofinish(op), ofcoast(op), 0);
				}
				else if (ucoast(up) == scoast1(getsector(map, ostart(op))))
				{				
					np = getneighbor(coast1neigh(getsector(map, ostart(op))), ofinish(op), ofcoast(op), 0);
				}
				else if (ucoast(up) == scoast2(getsector(map, ostart(op))))
				{				
					np = getneighbor(coast2neigh(getsector(map, ostart(op))), ofinish(op), ofcoast(op), 0);
				}
				else
				{				
					np = getneighbor(coast3neigh(getsector(map, ostart(op))), ofinish(op), ofcoast(op), 0);
				}
				if (np == NULL)
				{
					setofails(op, R_BADCOAST);
				}
				else
				{
					setofcoast(op, ncoast(np));
				}
             }
		 }
     }

     if (orderfails(op))
          return false;
     else
          return true;
}


/* do some sanity checks on the retreat */
void
checkretreat (Order rp)
{
     if (!isorder(rp))
          return;

     if (obadsyntax(rp))
          return;

     /* make sure the retreat is in the dislodge list */
     Dislodge* dp = dslglist.Find( ostart(rp) );
     if ( dp == NULL )
          setofails(rp, R_NSU);
         else if ( dp->Owner() == ocountry(rp))
         {
                if (otype(rp) == O_MOVE && !innlist(dp->Retreats(), ofinish(rp), ofcoast(rp), 0))
                        setofails(rp, R_FAILS);
         }

     /* set the unit */
     if (ounit(rp) == U_NON)
          setounit(rp, dp->Unit());

     /* is the coast right? */
     /* can't use checkcoast since there's no unit there... */
     /* have to check dislodge list instead */
	 (void) checkcoast(rp, (Unit) dp);
}


/* do some sanity checks on the build order */
void
checkbuild (Order op)
{
     short reg, coun;

     if (obadsyntax(op))
          return;

     switch (otype(op)) {
     case O_BUILD:
          reg = ostart(op);
          if (reg == 0)
                  return;
          coun = ocountry(op);

          /* clean up coast */
          /* being *real* nice here and sluffing off coast */
          if (ounit(op) == U_ARM || ounit(op) == U_WNG || !isbicoastal(getsector(map,reg)))
               setoscoast(op, C_NONE);

          /* if the coasts don't match up */
          if (ounit(op) == U_FLT && isbicoastal(getsector(map,reg)) && !matchcoast(oscoast(op), reg))
               setofails(op, R_BADCOAST);
		  /* if it is Störtebekerjäger in Hanse */
		  else if (gIsHanse && coun == 8)
		  {
				if (map[reg].unit != NULL)
				{
					setofails(op, R_CANTBUILD);
				}
				else if (!(issbj(getsector(map,reg)) && ounit(op) == U_FLT)) 
				{
					setofails(op, R_CANTBUILD);
				}
		  }
		  /* if it is a reserve build center under the ERM rule */
		  else if (gHasEmergencyRule 
					&& (countries[coun].numSibCenters > 0) 
					&& ourreservesupply(reg, coun)
					&& onehome(coun, supplies[coun])
					&& !allhome(coun, supplies[coun]))
		  {
				if (map[reg].unit != NULL)
				{
					setofails(op, R_CANTBUILD);
					break;
				}
		  }
          /* if it's not one of our home centers */
          else if (!(oursupply(reg, coun)
                                  || (gBuildMode == VBM_ABERRATION)
								  || (gBuildMode == VBM_CHAOS)
								  || haschaosbuildrule(getsector(map,reg)))
          /* or we don't own it, period */
              || !inslist(supplies[coun], reg)
          /* or we're trying to build a fleet on land */
              || (ounit(op) == U_FLT && !iscoastal(getsector(map,reg)))
          /* or there's a unit there already */
              || map[reg].unit != NULL)
          {
               setofails(op, R_CANTBUILD);
          }

          break;

     case O_REMOVE:
     case O_DEF_REM:
               reg = ostart(op);
               coun = ocountry(op);
          if (countries[coun].builds >= 0
                        || !inulist(units[coun], ounit(op), reg, C_NONE))
               setofails(op, R_NSU);

          break;

     default:
          break;

     }

}


int oursupply (short reg, short c)
{
     register short i;

     for (i = 0; i < countries[c].numCenters; i++) {
          if (countries[c].home[i] == reg)
               return 1;
     }

     return 0;
}

int ourreservesupply (short reg, short c)
{
     register short i;

     for (i = 0; i < countries[c].numSibCenters; i++) {
          if (countries[c].sib[i] == reg)
               return 1;
     }

     return 0;
}


/*
 * returns true if we own at least one home sector
 */
int onehome(short c, SupplyList& supply)
{
        register short i;

        for (i = 0; i < countries[c].numCenters; i++)
        {
                if (inslist(supply, countries[c].home[i]))
                        return 1;
        }
        return 0;
}

/*
 * returns true if we own all home sectors
 */
int allhome(short c, SupplyList& supply)
{
        register short i;

        for (i = 0; i < countries[c].numCenters; i++)
        {
                if (!inslist(supply, countries[c].home[i]))
                        return 0;
        }
        return 1;
}


int matchcoast (CoastType coast, short reg)
{
     if (map[reg].coast1 == coast)
          return 1;
     if (map[reg].coast2 == coast)
          return 1;

     return 0;
}


void
exec_orders(Orderlist orders[], bool moves)
{

        short numbuilds[MAX_COUN+1];
        if (dislodges > 0 && !moves)
                exec_retreats(orders, units, supplies, lands, dislodges, dslglist, adjustments, numbuilds);
        else if (adjustments && !moves)
                exec_builds(orders, units, supplies, dislodges, dslglist, adjustments, numbuilds);
        else
                exec_moves(orders, units, supplies, lands, dislodges, dslglist, adjustments, numbuilds);
		
    for (int i = 1; i <= NUM_COUN; ++i)
    {
             countries[i].builds = numbuilds[i];
    }

        nextseason();
}

void
exec_moves (Orderlist moves[], UnitList units[], SupplyList supplies[], SupplyList lands[], 
                        short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[])
{
     register Order op;
     register Unit up;
     register short i, sour, dest;

     for (i = 1; i <= NUM_COUN; i++)
     {
          for (op = startorder(moves[i]); isorder(op); nextorder(op))
          {
               if (!orderfails(op) && otype(op) == O_MOVE)
               {
                    sour = ostart(op);
                    dest = ofinish(op);
                    /* make sure we don't wipe out unit that's already moved *
                                           if (inulist(units[i], U_NON, sour, C_NONE)
                        && !movesto(moves[i], sour)) */
                    deleteunit(&units[i], sour);
                    /* just to be sure Armies have no coastal info */
                    if (ounit(op) == U_FLT)
                                    addunit(&units[i], ounit(op), dest, ofcoast(op), i);
                          else
                                    addunit(&units[i], ounit(op), dest, C_NONE, i);
               }
			   else if (!orderfails(op) && otype(op) == O_TRAFO)
			   {
					sour = ostart(op);
                    dest = ofinish(op);
					deleteunit(&units[i], sour);
                    if (ounit(op) == U_FLT)
                        addunit(&units[i], U_ARM, dest, C_NONE, i);
                    else
                        addunit(&units[i], U_FLT, dest, ofcoast(op), i);
			   }
          }
          unit_consist(units+i);
     }

     /* delete dislodged units */
     Dislodge* dp = dslglist.Start();
         while ( dp != dslglist.End() )
     {
                deleteunit(&units[dp->Owner()], dp->Sector());
        if (dp->Retreats() == NULL)
        {
                Dislodge* np = dslglist.Next(dp);
                (void) dslglist.Delete( dp->Sector() );
            dislodges--;
            dp = np;
              }
              else
              {
                      dp = dslglist.Next(dp);
              }
     }

     /* deal with new ownership */
     if (season == S_AUTUMN || (season == S_FALL && !dislodges))
     {
		  // mark all supply centers as unblocked
		  unblockallsupply(supplies);
          for (i = 1; i <= NUM_COUN; i++)
          {
			 for (up = startunit(units[i]); isunit(up); nextunit(up))
			   	    if (issupp(getsector(map, usector(up))))
					{
						if (!inslist(supplies[i], usector(up)))
						{
							if (utype(up) == U_WNG)
							{
								// SC blocked, no change of owner
								blocksupply(supplies, usector(up));   
							}    
							else
							{
								deleteallsupply(supplies, usector(up));
								if (gIsHanse && i == 8)
								{
									addsupply(supplies, usector(up), 0);
									// country 8 in Hanse does not gain SCs,
									// but makes it a neutral SC instead           
								}
								else
								{
									addsupply(supplies+i, usector(up), i);
								}
							}
						}
					}
					else if (island(getsector(map, usector(up))))
					{
						if (!inslist(lands[i], usector(up)))
						{
							if (!(utype(up) == U_WNG))
							{
                              deleteallsupply(lands, usector(up));
                              if (gIsHanse && i == 8)
								{
									addsupply(lands, usector(up), 0);
									// country 8 in Hanse does not gain SCs,
									// but makes it a neutral SC instead           
								}
								else
								{
									addsupply(lands+i, usector(up), i);
								}
							}
						}
					}
	      }
		  if (gIsHanse)
		  {
			    if (!gPreview)
				{
					if (CheckBlockAction("Shall Moscow be blocked?", "Blockade"))
					{
						short sector;
						string2sector(&sector, "Mos");
						blocksupply(supplies, sector);
					}
				}
				if (year >= 1401)
				{
					// Elimination of country 8 (Störtebekerjäger)
					deleteallsupply(supplies, countries[8].home[0]);
				}
		  }
		  adjustments = countbuilds(units, supplies, dslglist, numbuilds);
     }
     else
             (void) countbuilds(units, supplies, dslglist, numbuilds);
}


/* perform the disband or build, depending */
void
exec_builds(Orderlist orders[], UnitList units[], SupplyList supplies[],
                        short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[])
{
     register short i, reg;
     Order op;

     for (i = 1; i <= NUM_COUN; i++)
     {
          for (op = startorder(orders[i]); isorder(op); nextorder(op)) {
               reg = ostart(op);
               if (otype(op) == O_BUILD && !orderfails(op))
               {
                    if (ounit(op) == U_FLT)
                                    addunit(units+i, ounit(op), reg, oscoast(op), i);
                          else
                                    addunit(units+i, ounit(op), reg, C_NONE, i);
               }
               else if ((otype(op) == O_REMOVE || otype(op) == O_DEF_REM)
                                       && !orderfails(op))
               {
                               deleteunit(units+i, reg);
               }
          }
          /* any builds missing are ignored */
          countries[i].builds = 0;
     }

     /* delete disbanded units */
     if (dislodges < 0)
     {
                for (Dislodge* dp = dslglist.Start(); dp != dslglist.End(); dp = dslglist.Next(dp) )
                {
                       deleteunit(&units[dp->Owner()], dp->Sector());
                   }
            }

     dislodges = 0;
     dslglist.Clean();
     adjustments = 0;
     (void) countbuilds(units, supplies, dslglist, numbuilds);
}


/* returns number of builds/disbands needed for this year */
/* modifies build list for countries */
int
countbuilds(UnitList units[], SupplyList supplies[], const DislodgeList& dslglist, short builds[])
{
     short count, c, j, uc, sc, bc, rc, supp;
     Supply sp;

     count = 0;
     for (c = 1; c <= NUM_COUN; c++)
     {
          uc = ulistsize(units[c]);
          /* catch any dislodged units */
          for (Dislodge* dp = dslglist.Start(); dp != dslglist.End(); dp = dslglist.Next(dp) )
                           if (dp->Owner() == c)
                                     uc++;
          sc = slistsize(supplies[c]);
		  rc = 0;
		  if (gHasEmergencyRule)
		  {
			// reserve centers and alternative building sites
			if (countries[c].numSibCenters > 0 && onehome(c, supplies[c]) && !allhome(c, supplies[c]))
				rc++;
		  }
		  bc = countblocked(supplies[c]);
          builds[c] = sc+rc-uc-bc;

          if (builds[c] > 0)
          {
             /* can build anywhere we have a free supply center */
             /* (for Aberration, as long as we have at least one home sector) */
            sc = 0;
            if (((gBuildMode == VBM_ABERRATION) && onehome(c, supplies[c]))
                          || (gBuildMode == VBM_CHAOS))
            {
                    for (sp = startsupply(supplies[c]); issupply(sp); nextsupply(sp))
                          {
                                     if (!inulist(units[c], U_NON, ssector(sp), C_NONE))
                                         sc++;
                          }
            }
            /* can only build in free home centers or special Hanse-Centers */
            else if (gBuildMode == VBM_HANSE)
            {
            
				for (sp = startsupply(supplies[c]); issupply(sp); nextsupply(sp))
                {
					if (!inulist(units[c], U_NON, ssector(sp), C_NONE) 
						&& (oursupply(ssector(sp),c) 
								|| haschaosbuildrule(getsector(map,ssector(sp)))))
					{
                        sc++;
					}
			    }
			}
			else
			{
				for (j = 0; j < countries[c].numCenters; j++)
				{
					if ((supp = countries[c].home[j])
						&& inslist(supplies[c], supp)
						&& (map[supp].unit == NULL))
						sc++;
				}
				if (rc > 0)
				{
					for (j = 0; j < countries[c].numSibCenters; j++)
					{
						if ((supp = countries[c].sib[j])
							&& (map[supp].unit == NULL))
							sc++;
					}
				}
			}
			 if (sc < builds[c])
                builds[c] = sc;
          }
          count += ((builds[c] < 0) ? -(builds[c]) : builds[c]);
     }

     return count;

}

/* returns number of support centers blocked by wings */
int
countblocked(SupplyList supplies)
{
	
	Supply sp;
	short count;
	
	count = 0;
	
	for (sp = startsupply(supplies); issupply(sp); nextsupply(sp))
				{
					if (isblocked(sp))
						count++;
	                // someone else's wing has blocked the sc
					// or Moscow is blocked in Hanse
				}

	return count;
}

void
exec_retreats (Orderlist orders[], UnitList units[], SupplyList supplies[], SupplyList lands[],
                                short& dislodges, DislodgeList& dslglist, short& adjustments, short numbuilds[])
{
     Order op;
     Unit up;
     register short i, sour, dest;

     for (i = 1; i <= NUM_COUN; i++) {
          for (op = startorder(orders[i]); isorder(op); nextorder(op))
          {
               if (otype(op) == O_MOVE && !orderfails(op))
               {
                    sour = ostart(op);
                    dest = ofinish(op);
                    if (ounit(op) == U_FLT)
                                    addunit(units+i, ounit(op), dest, ofcoast(op), i);
                          else
                                    addunit(units+i, ounit(op), dest, C_NONE, i);
               }
          }
     }
     dislodges = 0;
     dslglist.Clean();

     /* deal with new ownership */
     if (season == S_AUTUMN || season == S_FALL)
	 {
		  // mark all supply centers as unblocked
		  unblockallsupply(supplies);
          for (i = 1; i <= NUM_COUN; i++)
          {
               for (up = startunit(units[i]); isunit(up); nextunit(up))
                    if (issupp(getsector(map, usector(up))))
					{
                         if (!inslist(supplies[i], usector(up)))
                         {
                              if (utype(up) == U_WNG)
							  {
								   blocksupply(supplies, usector(up));
							  }
							  else
                              {
									deleteallsupply(supplies, usector(up));
									if (gIsHanse && i == 8)
									{
										addsupply(supplies, usector(up), 0);
										// country 8 in Hanse does not gain SCs,
										// but makes it a neutral SC instead           
									}
									else
									{
										addsupply(supplies+i, usector(up), i);
									}
						      }
                         }
					}
					else if (island(getsector(map, usector(up))))
					{
                         if (!inslist(lands[i], usector(up)))
                         {
                              if (!(utype(up) == U_WNG))
                              {
                                    deleteallsupply(lands, usector(up));
                                    if (gIsHanse && i == 8)
									{
										addsupply(lands, usector(up), 0);
										// country 8 in Hanse does not gain SCs,
										// but makes it a neutral SC instead           
									}
									else
									{
										addsupply(lands+i, usector(up), i);
									}
                              }
                         }
					}
          }
		  if (gIsHanse)
		  {
			    if (!gPreview)
				{
					if (CheckBlockAction("Shall Moscow be blocked?", "Blockade"))
					{
						short sector;
						string2sector(&sector, "Mos");
						blocksupply(supplies, sector);
					}
				}
				if (year >= 1401)
				{
					// Elimination of country 8 (Störtebekerjäger)
					deleteallsupply(supplies, countries[8].home[0]);
				}
		  }
		  adjustments = countbuilds(units, supplies, dslglist, numbuilds);
     }
     else
             (void) countbuilds(units, supplies, dslglist, numbuilds);
}


// count units, supplies, builds
void
sim_next_phase( UnitList newunits[], SupplyList newsupplies[], SupplyList newlands[],
                                short numunits[], short numsupplies[], short numlands[],
                                short& newdislodges, DislodgeList& newdslglist, short& newadjustments,
                                short numbuilds[], bool moves )
{
        // first, do initial setup
        short c;
		gPreview = true;
        newdislodges = dislodges;
        newadjustments = adjustments;
        for (c = 0; c <= NUM_COUN; ++c)
        {
                // copy units
                initulist(newunits[c]);
                for (Unit u = startsupply(units[c]); isunit(u); nextunit(u))
                {
                        addunit(&newunits[c], utype(u), usector(u), ucoast(u), c);
                }
                // copy supplylists
                initslist(newsupplies[c]);
                for (Supply s = startsupply(supplies[c]); issupply(s); nextsupply(s))
                {
                        addsupply(&newsupplies[c], ssector(s), c);
						if (isblocked(s))
						{
							blocksupply(newsupplies, ssector(s));
						}
                }
				initslist(newlands[c]);
                for (Supply s = startsupply(lands[c]); issupply(s); nextsupply(s))
                {
                        addsupply(&newlands[c], ssector(s), c);
                }
        }
        // copy dislodges
        newdslglist = dslglist;

        // pretend to do the commit
        if (newdislodges > 0 && !moves)
                exec_retreats(orders, newunits, newsupplies, newlands, newdislodges, newdslglist, newadjustments, numbuilds);
        else if (newadjustments && !moves)
                exec_builds(orders, newunits, newsupplies, newdislodges, newdslglist, newadjustments, numbuilds);
        else
                exec_moves(orders, newunits, newsupplies, newlands, newdislodges, newdslglist, newadjustments, numbuilds);

    // cleanup
        for (c = 1; c <= NUM_COUN; ++c)
        {
                numunits[c] = ulistsize(newunits[c]);
                numsupplies[c] = ulistsize(newsupplies[c]);
				numlands[c] = ulistsize(newlands[c]);
        }
        // make sure map is consistant with original units
        unitmapconsist();
		gPreview = false;
}


bool
OrderToRect(Order op, Rect *theRect)
{
        Point pt;

        switch (otype(op))
        {
        case O_MOVE:
                return MoveToRect(op, theRect);
                break;

        case O_SUPP:
                return SupportToRect(op, theRect);
                break;

        case O_CONV:
                return ConvoyToRect(op, theRect);
                break;

        case O_BUILD:
        case O_REMOVE:
        case O_DEF_REM:
        case O_DISBAND:
        case O_NMR_DIS:
        case O_HOLD:
        default:
                SetRect(theRect, -16, -16, 16, 16);
                SectorToPoint(&pt, ostart(op), oscoast(op));
                OffsetRect(theRect, pt.h, pt.v);
                return true;
                break;
        }
}


bool
MoveToRect(Order order, Rect *theRect)
{
        Point pt1, pt2;

        if (neighborsector(ounit(order), ostart(order),
                                        oscoast(order), ofinish(order), ofcoast(order), false))
        {
                SectorToPoint(&pt1, ostart(order), oscoast(order));
                SectorToPoint(&pt2, ofinish(order), ofcoast(order));

                Pt2Rect(pt1, pt2, theRect);
                InsetRect(theRect, -16, -16);

                return true;
        }
        else /* might be a convoy, must redraw whole screen */
                return false;
}

bool
SupportToRect(Order order, Rect *theRect)
{
        Point pt1, pt2, pt3;

        SectorToPoint(&pt1, ostart(order), oscoast(order));
        SectorToPoint(&pt2, ostart(osuborder(order)), oscoast(osuborder(order)));
        if (otype(osuborder(order)) == O_MOVE)
        {
                SectorToPoint(&pt3, ofinish(osuborder(order)), ofcoast(osuborder(order)));
                pt2.h = (pt2.h + pt3.h)/2;
                pt2.v = (pt2.v + pt3.v)/2;
        }
        Pt2Rect(pt1, pt2, theRect);
        InsetRect(theRect, -16, -16);

        return true;
}

bool
ConvoyToRect(Order, Rect *)
{
        return false;

}

