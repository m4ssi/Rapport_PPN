#include <regex>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

extern "C"	{
	#include <ctype.h>
	#include <unistd.h>
}

// Ouvre un pipe sur la sortie d'une commande
// \param	cmd		commande à exécuter
// \return			descripteur de fichier (C FILE *)
FILE * open_pipe ( const char * cmd)	{
	FILE * fd = nullptr;
    if( ( fd = popen ( cmd, "r")) == nullptr)	{
        perror( "popen : ");
        exit(2);
    }	
    return fd;
}

// Ferme un pipe sur la sortie d'une commande
// \param	fd		descripteur de fichier (C FILE *)
void close_pipe ( FILE * fd)	{
	pclose( fd);
}

// Lit la sortie d'une commande et la stock ligne par ligne dans un vector
// \param	fd		descripteur de fichier du fichier à lire
// \ret				std::vector<std::string> contenant la sortie de la commande
std::vector<std::string> read_data ( FILE * fd)	{
	std::vector<std::string> op;
	char buf[150] = "\0";
    
    while( /*( fread( &buf, sizeof(char), 1, fd) != 0) &&*/ (!feof( fd)))	{
		if (fgets ( buf, 150, fd))
			op.push_back( buf);
	}
	return op;
}

// Concatène les arguments de maqao pour en faire une commande
// \param	argc	nombre d'arguments du programme
// \param	argv	liste des arguments du programme
// \ret				chaine de caractère contenant la commande à executer
std::string get_cmd ( int argc, char ** argv)	{
	std::string cmd = "maqao oneview ";
	for (int i = 1; i < argc; i++)	{
		cmd += argv[i];
		cmd += ' ';
	}
	return cmd;	
}

// Affiche la sortie textuelle de maqao
// \param	op	vecteur contenant les lignes de la sortie de maqao
// \ret			contenu du run à formater en orgmode
std::vector<std::string> print_stat ( std::vector<std::string>& op)	{
	// Indicateur de début de fichier
	std::string beg = "+==================================================================================================+\n";
    // Affichage de la sortie de maqao 
    while( op[0] != beg)	{
		std::cout << op[0];
		op.erase ( op.begin());
	}
	bool out = true;
	unsigned to_print = 0;
	
	// Creation d'un vecteur contenant uniquement les informations a formater
	std::vector<std::string> file;
	for ( unsigned i = 0; i < op.size() && out; i++)	{
		if ( op[i].substr( 0,5) == "Info:")	{
			out = false;
			to_print = i;
		}
		else	{
			file.push_back( op[i]);
		}
	}

	// Affichage des messages de fin d'exécution de maqao
	for ( unsigned i = to_print; i < op.size(); i++)	{
		std::cout << op[i];
	}
	return file;
}


// Recherche la position du premier caractère ':' dans une chaine de caractère
// Renvoie -1 s'il n'y a pas de ':'
// \param 		s	chaine de caractère à lire
// \ret				position de la première occurence 
int first_double_dot ( std::string& s)	{
	for ( unsigned i = 0; i < s.size(); i++)	{
		if ( s[i] == ':')	{
			return (int) i;
		}
	}
	return -1;
}

// Recherche la position du dernier plus d'une ligne
// \param 		s	chaine de caractère à lire
// \ret				position de la dernière occurence 
int last_plus ( std::string& s)	{
	for ( int i = (int) s.size()-1; i > -1; i--)	{
		if ( s[i] == '+')	{
			return (int) i;
		}
	}
	return -1;
}

// Retourne le bon format de titre d'une sous-section de CQA
// \param	title	titre tel que l'affiche maqao
// \ret				titre en orgmode
std::string cqa_titles ( std::string& title)	{
	int hyphen_pos = 0,
		last_alpha = 0,
		n_dots = 0,
		t_size = title.size();
	// On compte le nombre de . pour ajouter le bon nombre de '*' et on cherche le début du titre
	for ( int i = 0; i < t_size && ( hyphen_pos == 0); i++)	{
		if ( title[i] == '.')	n_dots++;
		if ( title[i] == '-')	hyphen_pos = i;
	}
	// Cas d'un sous-titre (généralement ** Loop)
	if ( n_dots == 1)	{
		for ( int i = hyphen_pos+2; i < t_size && (last_alpha == 0); i++)	{
			if ( title[i+1] == ' ' && title[i+2] == ' ')	last_alpha = i;
		}
		return "**  "+title.substr(hyphen_pos+2, (last_alpha-(hyphen_pos+2)))+"\n";
	}
	// Ajout du nombre d'étoile
	std::string r;
	for (int i = 0; i < n_dots+1; i++)	{
		r += '*';
	}
	return r+title.substr(hyphen_pos+2);
}

// Lit dans un vecteur de string le contenu qui se trouve de la position de beg+pos_beg a la position de end+pos_end
// Le contenu lu est formaté en orgmode et stocké dans un vecteur
// \param	title		titre de la section a ecrire
// \param	m_title		titre de la section a chercher en premier
// \param	beg			chaine de caractère de début a chercher a parmier de m_title
// \param	end			chaine de caractère de fin de la section
// \param	input		vecteur contenant la sortie de maqao
// \param	output		vecteur contenant la sortie de maqao en orgmode
// \param	pos_beg		nombre de sauts a faire depuis beg
// \param	pos_end		nombre de sauts a faire depuis end
// \param	t_end		indique la présence de tableaux ressemblant au orgmode legerà ceux d'orgmode
void global_sub_sub_section ( std::string title, std::string m_title, std::string beg, std::string end, std::vector<std::string>& input, std::vector<std::string>& output, int pos_beg = 0, int pos_end = 0, bool t_end = false) {
	int i = 0,
		j = 0,
		k = 0;
	title += "\n\n";
	output.push_back( title);
	unsigned s_beg = beg.size(),
			 s_end = end.size(),
			 s_title = m_title.size();
	while ( input[i].substr(0, s_beg) != beg)		{		i++;}	k = i-3;
	while ( input[k].substr(0, s_title) != m_title)	{		k++;}	j = k;
	while ( input[j].substr(0, s_end) != end)		{		j++;}
	output.push_back("|-\n");
	for (int it = k+pos_beg; it < j+pos_end; it++)	{
		int dots = first_double_dot ( input[it]);
		if ( dots > 0)	{
			input[it][0] = '|';
			input[it].pop_back();
			input[it].push_back('|');
			input[it].push_back('\n');
			input[it][dots] = '|';
			output.push_back( input[it]);	
		}
		else if ( dots == -1 && input[it].at(0) == '\n')	{
			output.push_back("|-\n");			
		}
		else if (dots == -1 && t_end)	{
			input[it][0] = '|';
			output.push_back( input[it]);
		}
		else	{
			std::cout << "Error paring : "<< title << std::endl;
			std::cout << "             : "<< input[it] << std::endl;
		}
	}
	output.push_back("|-\n\n");
}

// Lit dans un vecteur de string le contenu qui se trouve de la position de beg+pos_beg a la position de end+pos_end
// Le contenu lu est formaté en orgmode et stocké dans un vecteur
// \param	title		titre de la section
// \param	beg			chaine de caractère de début de la section
// \param	end			chaine de caractère de fin de la section
// \param	input		vecteur contenant la sortie de maqao
// \param	output		vecteur contenant la sortie de maqao en orgmode
// \param	pos_beg		nombre de sauts a faire depuis beg
// \param	pos_end		nombre de sauts a faire depuis end
// \param	t_end		indique la présence de tableaux ressemblant au orgmode legerà ceux d'orgmode
void global_sub_section ( std::string title, std::string beg, std::string end, std::vector<std::string>& input, std::vector<std::string>& output, int pos_beg = 0, int pos_end = 0, bool t_end = false) {
	int i = 0,
		j = 0;
	title += "\n\n";
	output.push_back( title);
	unsigned s_beg = beg.size(),
			 s_end = end.size();
	while ( input[i].substr(0, s_beg) != beg)	{		i++;}	j = i;
	while ( input[j].substr(0, s_end) != end)	{		j++;}
	output.push_back("|-\n");
	for (int it = i+pos_beg; it < j+pos_end; it++)	{
		int dots = first_double_dot ( input[it]),
			plus = last_plus ( input[it]);
		if ( dots > 0)	{
			input[it][0] = '|';
			input[it].pop_back();
			input[it].push_back('|');
			input[it].push_back('\n');
			input[it][dots] = '|';
			output.push_back( input[it]);
		}
		else if ( dots == -1 && input[it].at(0) == '\n')	{
			output.push_back("|-\n");			
		}		
		else if (dots == -1 && t_end)	{
			if ( plus == (int) (input[it].size()-2))	{
				input[it][0] = '|';
				input[it][1] = '-';
				input[it].pop_back();
				input[it].pop_back();
				input[it].push_back('|');
				input[it].push_back('\n');
				output.push_back( input[it]);				
			}
			else	{
				input[it][0] = '|';
				output.push_back( input[it]);
			}
		}
		else	{
			std::cout << "Error paring : "<< title << std::endl;
			std::cout << "             : "<< input[it] << std::endl;
		}
	}
	output.push_back("|-\n\n");
}

// Convertie la sortie de maqao dans format textuel au orgmode et le stock dans un vecteur
// Les fichiers généras par maqao semblent avoir la même structure et cette structure a été exploité pour imaginer ce parseur
// \param		f		vecteur contenant la sortie au format text
// \ret					vecteur contenant le fichier orgmode à écrire
std::vector<std::string> txt_to_org ( std::vector<std::string>& f)	{

	// Global
	std::vector <std::string> glob;
	glob.push_back("*  GLOBAL\n\n");
	// 		Experiment Summary 
	global_sub_section ( "**  Experiment Summary", "  Application:", "  MAQAO build:", f, glob, 0, 1);
	// 		Global Metrics	
	global_sub_section ( "**  Global Metrics", "  Total Time:", "  Array Access Efficiency:", f, glob, 0, 1);
	global_sub_sub_section ( "***  If No Scalar Integer:", "  If No Scalar Integer:" , "      Potential Speedup:", "      Nb Loops to get 80%:", f, glob, 1, 1);
	global_sub_sub_section ( "***  If FP Vectorized:", "  If FP Vectorized:","      Potential Speedup:", "      Nb Loops to get 80%:", f, glob, 1, 1);
	global_sub_sub_section ( "***  If Fully Vectorized:","  If Fully Vectorized:" ,"      Potential Speedup:", "  Perfect OpenMP + MPI + Pthread:", f, glob, 1, 1);
	//		Potential Speedups
	glob.push_back("**  Potential Speedups\n\n");
	global_sub_sub_section ( "***  If No Scalar Integer:", "  If No Scalar Integer:","      Number of loops", "      Cumulated Speedup", f, glob, 1, 1, true);
	global_sub_sub_section ( "-  Top 5 loops:", "  If No Scalar Integer:", "  Top 5 loops:", "\n", f, glob, 4, 0);
	global_sub_sub_section ( "***  If FP Vectorized:", "  If FP Vectorized:", "      Number of loops", "      Cumulated Speedup", f, glob, 1, 1, true);
	global_sub_sub_section ( "-  Top 5 loops:", "  If FP Vectorized:", "  Top 5 loops:", "\n", f, glob, 4, 0);
	global_sub_sub_section ( "***  If Fully Vectorized:", "  If Fully Vectorized:", "      Number of loops", "      Cumulated Speedup", f, glob, 1, 1, true);	
	global_sub_sub_section ( "-  Top 5 loops:", "  If Fully Vectorized:", "  Top 5 loops:", "\n", f, glob, 4, 0);	
	
	// Application
	glob.push_back("*  APPLICATION\n\n");
	global_sub_section ( "**  Categorization", "   Category ", "   Time (%) ", f, glob, 0, 1, true);
	global_sub_section ( "**  Function Based Profiling", "   Buckets", "\n", f, glob, 0, 0, true);
	

	global_sub_section ( "**  Loop Based Profiling", "   Buckets", "\n", f, glob, 0, 0, true);
	
	// Functions
	glob.push_back("*  FUNCTIONS\n\n");
	global_sub_section ( "**  Top 10 Functions", "   Function", "\n", f, glob, 0, 0, true);
	
	// Loops
	glob.push_back("*  LOOPS\n\n");
	global_sub_section ( "**  Top 10 Loops", "   Loop Id", "\n", f, glob, 0, 0, true);
	
	// CQA
	glob.push_back("*  CQA\n\n");

	int i_cqa = 0;
	while ( f[i_cqa].substr(0, 45) != "+                                          5."){ i_cqa++;}
	bool to_print = true;
	// Copie du contneu de CQA en orgmode
	for ( int i = i_cqa; i < (int) f.size() && to_print; i++)	{
		if ( f[i].substr(0, 5) == "Info:" || f[i].substr(0, 5) == "* War") to_print = false;
		else if (   f[i].substr(0, 45) == "+                                          5." ||
					f[i].substr(0, 8) == "      5.")	{
			glob.push_back ( cqa_titles ( f[i]));
			i++;
		}
		else 	{
			glob.push_back( f[i]);
		}
	}
	return glob;
}


std::string org_filename ( int argc, char ** argv)	{
	std::string f_name;
	int i_xp = 0,
		i_bin = 0;
	bool format = false;
	for (int i = 1; i < argc; i++)	{
		std::string tmp (argv[i]);
		if ( tmp.substr(0,3) == "-xp" || tmp.substr(0,17) == "--experiment-path")
			i_xp = i;
		if ( tmp.substr(0,8) == "--binary" )
			i_bin = i;
		if ( tmp == "-of=all" || tmp == "-of=text" || tmp == "--output-format=all"  || tmp == "--output-format=text")	{
			format = true;
		}
	}
	if ( !format)	{
		std::cout << "Output format must be : all, text !" << std::endl;
		exit(1);
	}
	if ( i_xp)	{
		int xp_deb = (argv[i_xp][1] == '-' ? 18 : 4);
		std::string	op_xp(argv[i_xp]),
					op_bin(argv[i_bin]);
		f_name = op_xp.substr ( xp_deb) + "/RESULTS/" + op_bin.substr(9)+ "_one.org";
	}
	else	{
		f_name = "output.org";
	}
	return f_name;	
}

void write_org_file ( std::string& path, std::vector<std::string> org)	{
	std::ofstream os;
	os.open ( path);
	for (int i = 0; i < (int) org.size(); i++)
		os << org[i];
	os.close();	
}

std::vector<std::string> read_output ( std::string cmd)	{
	FILE * fd = open_pipe ( cmd.c_str());
	std::vector<std::string> output = read_data ( fd);
	close_pipe ( fd);
	return output;
}

int main ( int argc, char ** argv)	{
	


	std::string f_name = org_filename ( argc, argv);

	std::string cmd = get_cmd ( argc, argv);
	
	std::vector<std::string> output = read_output ( cmd);
	
	std::vector<std::string> file = print_stat ( output);
	
	std::vector<std::string> org = txt_to_org ( file);

	write_org_file ( f_name, org);
	
	return 0;
}
