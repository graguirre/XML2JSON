/**
 * section: Tree
 * synopsis: Navigates a tree to convert into json format
 * purpose: Parse a file to a tree, use xmlDocGetRootElement() to
 *          get the root element, then walk the document and print
 *          all the element name in document order.
 * usage: tree1 filename
 * test: tree test.xml > test.json
 * authors: Gonzalo Aguirre / JL
 */
#include <stdio.h>
#include <ctype.h>	/* iscntrl() */
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifdef LIBXML_TREE_ENABLED

static int empty=0; /* empty elem <e/> */

/**
 * To compile this file using gcc you can type
 * gcc `xml2-config --cflags --libs` -o tree tree.c
 */


/**
 * print_element_attrib:
 * @a_node: xml node to print attributes
 *
 * Print attribute/s of a given XML node
 */
static void
print_element_attrib (xmlNode * a_node,int indent)
{

    xmlAttr *cur_attr = NULL;
    xmlChar *cur_char = NULL;
    int attrib=0; 	/* element without attrib */

    printf(" %*s",indent,"");		/* indent */
    if(!a_node->children && !empty) 
	printf("{");  /* leaf */

    for (cur_attr = a_node->properties; cur_attr; cur_attr = cur_attr->next){
	attrib=1;		/* element with attrib */
        cur_char = xmlNodeListGetString(a_node->doc,cur_attr->children,1); /* getAttrib*/
	printf("\"%s\": \"%s\"", cur_attr->name, cur_char);
	if (cur_attr->next) printf(","); /*check if it's the last attrib*/
    }

    if(!a_node->children && !empty) 
	printf("}");  /* leaf */
    else
	empty = 0;	/* reset empty flag */
    
    if(attrib) printf(","); 		/* ends with , */
}

/**
 * check_group_element:
 * @a_node: first sibling xml node
 *
 * Compares xml element names with siblings
 * to group them.
 */
static int
check_group_element (xmlNode * a_node)
{
    int count=0,flag=1; /* xml elements name counter, first time flag */
    xmlNode *cur_node = NULL;
    xmlChar *cur_name = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) 
        if (cur_node->type == XML_ELEMENT_NODE){ 
	    if (flag) {
       	        cur_name = (xmlChar *) cur_node->name;
		flag = 0;
            }
	    if (cur_name == cur_node->name)
		count++;
	}

    return count;
}

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node,int indent)
{
    xmlNode *cur_node = NULL;
    xmlChar *cur_cont = NULL;
    int group = 0 /* group [ ... ] */,
	flag = 1  /* first elem group */;

    if (check_group_element(a_node) > 1) /* group tag? */
	group = 1; /* group elements' tag, just show the first one */

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
	    /* xml node name */
	    if (group && flag) { 	/* first xml node group */
       	        printf("\n%*s \"%s\": [\n %*s",indent, "", cur_node->name,indent+3,"");
		flag = 0;
	    } else if (group && !flag){ /* second and so on, group*/
       	        printf("\n%*s ",indent+3);
	    } else 			/* non grouped xml nodes */
       	        printf("\n%*s \"%s\": ",indent,"", cur_node->name);

	    /* xml node content */		
	    if (cur_node->children) 	/* check children not null */
		cur_cont = cur_node->children->content;
	    else {
		if(cur_node->properties) /*empty elem with or without attrib*/
			printf("\n"/*,cur_node->properties->doc->name*/);
		else {
			printf("null");
			if(cur_node->next->next) /* last element */
			    printf(",");
			empty = 1;	/* empty elem consider */
		}
		cur_cont = NULL;
	    }

	    if (cur_cont)		/* check content not null */
        	if(!iscntrl(cur_cont[0])){
		    printf("\"%s\"", cur_cont);
		    if(cur_node->next->next)	/* last element tag */
		    	printf(",");
		}

	    /* xml node attrib */	
        
	if(cur_node->children)
		if(iscntrl(cur_node->children->content[0]))
	 		printf("\n%*s{\n",indent,""); 		/* open element */

	print_element_attrib(cur_node,indent+3);	/* print attrib */

        print_element_names(cur_node->children,indent+3); 
	if(cur_node->children) 
		if(iscntrl(cur_node->children->content[0])){
			printf("\n%*s}",indent,""); /* close element */
		    	if(cur_node->next && cur_node->next->next)	/* last element tag */
		    		printf(",");
			
		}
	}
	if(!group && cur_node!=a_node) printf(" ");
    }
    if(group)
	printf("\n%*s" "]\n",indent,"");
}


/**
 * Simple example to parse a file called "file.xml", 
 * walk down the DOM, and print the name of the 
 * xml elements nodes.
 */
int
main(int argc, char **argv)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    if (argc != 2)
        return(1);

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(argv[1], NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", argv[1]);
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    printf("{"); /* principal curly braces */
    print_element_names(root_element,0);
    printf("\n}");

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return 0;
}
#else
int main(void) {
    fprintf(stderr, "Tree support not compiled in\n");
    exit(1);
}
#endif
