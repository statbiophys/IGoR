/*
 * Model_Parms.cpp
 *
 *  Created on: 3 nov. 2014
 *      Author: marcou
 *
 *      This class defines a Model_Parms object. In our framework this will be used
 *      to easily define the inferred model, by using a directed graph structure.
 *      This helps defining conditional marginals in the model.
 */

#include "Model_Parms.h"
using namespace std;

/*
 * Default constructor, creates an empty Model_Parms
 */
Model_Parms::Model_Parms() {
	this->events = *(new list <shared_ptr<Rec_Event>>()); //FIXME nonsense new
	this->edges = *(new unordered_map <Rec_Event_name , Adjacency_list >());

}

/*
 * Construct a Model_Parms from a list of event and initialize connectivities/Adjacency_lists
 */
Model_Parms::Model_Parms(list <shared_ptr<Rec_Event>> event_list){
	this->events = event_list;
	this->edges = *(new unordered_map<Rec_Event_name,Adjacency_list>()); //FIXME nonsense new
	size_t event_identifier = 0;
	for(list<shared_ptr<Rec_Event>>::const_iterator iter = this->events.begin() ; iter != this->events.end() ; ++iter){
		this->edges.emplace( (*iter)->get_name() ,*(new Adjacency_list()));
		(*iter)->set_event_identifier(event_identifier);
		++event_identifier;
	}
}

/*
 * Provide a deep copy constructor of the Model_Parms object
 * This method will make a copy of all the contained events and error rate as well
 */
Model_Parms::Model_Parms(const Model_Parms& other){
	for(list<shared_ptr<Rec_Event>>::const_iterator iter = other.events.begin() ; iter!= other.events.end() ; ++iter){
		this->events.push_back((*iter)->copy());
	}

	for(unordered_map <Rec_Event_name , Adjacency_list >::const_iterator iter = other.edges.begin() ; iter != other.edges.end() ; ++iter){
		Adjacency_list adjacency_list;
		//TODO very dirty need to be changed
		for(list<shared_ptr<Rec_Event>>::const_iterator jiter = (*iter).second.children.begin() ; jiter != (*iter).second.children.end() ; ++jiter){
			for(list<shared_ptr<Rec_Event>>::const_iterator kiter = this->events.begin() ; kiter != this->events.end() ; kiter++){
				if((**kiter)==(**jiter)){
					adjacency_list.children.push_back((*kiter));
					break;
				}
			}
		}
		for(list<shared_ptr<Rec_Event>>::const_iterator jiter = (*iter).second.parents.begin() ; jiter != (*iter).second.parents.end() ; ++jiter){
			for(list<shared_ptr<Rec_Event>>::const_iterator kiter = this->events.begin() ; kiter != this->events.end() ; ++kiter){
				if((**kiter)==(**jiter)){
					adjacency_list.parents.push_back((*kiter));
					break;
				}
			}
		}

		this->edges.emplace((*iter).first,adjacency_list);
	}
	this->error_rate = other.error_rate->copy();
}




/*
 * Provide a deep copy of the model parameters
 */
/*Model_Parms::Model_Parms(const Model_Parms& other){
	this->error_rate = other.error_rate->copy();
	for(unordered_map<Rec_Event_name , Adjacency_list>::const_iterator iter=other.edges.begin() ; iter != other.edges.end();iter++){
		this->edges.emplace((*iter).first,*iter.)
	 }

	for(list<Rec_Event*>::const_iterator iter = other.events.begin() ; iter != other.events.end() ; iter++){
		this->events.emplace_back((*iter)->copy());
	}
}*/


/*
 * Avoid memory leaks by deleting all events
 */
Model_Parms::~Model_Parms() {
/*	for(list<Rec_Event*>::iterator iter = events.begin() ; iter != events.end() ; iter++){
		delete (*iter);
	}
	delete error_rate;*/
}



/*
 * Add an event to the event list, and initialize its connectivity
 */
bool Model_Parms::add_event(shared_ptr<Rec_Event> event_point){
	//this->events.push_back(event_point);
	event_point->set_event_identifier(this->events.size());
	this->events.emplace_back(event_point);
	this->edges.emplace(event_point->get_name(),*(new Adjacency_list())); //FIXME nonsense new
	return 1;
}
bool Model_Parms::add_event(Rec_Event* event_point){
	return this->add_event(shared_ptr<Rec_Event>(event_point,null_delete<Rec_Event>()));
}

//TODO add remove edge method


/*
 * Get the list of parents of the given Rec_event
 */
list<shared_ptr<Rec_Event>> Model_Parms::get_parents(Rec_Event* event) const{
	return edges.at(event->get_name()).parents;
}
list<shared_ptr<Rec_Event>> Model_Parms::get_parents(shared_ptr<Rec_Event> event) const{
	return edges.at(event->get_name()).parents;
}


/*
 * Get the list of children of the given Rec_event
 */
list<shared_ptr<Rec_Event>> Model_Parms::get_children(Rec_Event* event) const{
	return edges.at(event->get_name()).children;
}
list<shared_ptr<Rec_Event>> Model_Parms::get_children(shared_ptr<Rec_Event> event) const{
	return edges.at(event->get_name()).children;
}


/*
 * Add a directed edge between two events
 */
bool Model_Parms::add_edge(Rec_Event* parent_point, Rec_Event* child_point){
	//TODO check whether these events exist
	this->edges.at( parent_point->get_name() ).children.push_back(shared_ptr<Rec_Event>(child_point,null_delete<Rec_Event>()));
	this->edges.at( child_point->get_name() ).parents.push_back(shared_ptr<Rec_Event>(parent_point,null_delete<Rec_Event>()));
	return 1;
}
bool Model_Parms::add_edge(shared_ptr<Rec_Event> parent_point, shared_ptr<Rec_Event> child_point){
	//TODO check whether these events exist
	this->edges.at( parent_point->get_name() ).children.push_back(child_point);
	this->edges.at( child_point->get_name() ).parents.push_back(parent_point);
	return 1;
}



/*
 * This method returns all the roots of the tree (events with no parents).
 */
list<shared_ptr<Rec_Event>> Model_Parms::get_roots() const{
	list<shared_ptr<Rec_Event>> root_list = *(new list<shared_ptr<Rec_Event>>); //FIXME nonsense new
	for ( list <shared_ptr<Rec_Event>>::const_iterator iter = this->events.begin(); iter != this->events.end(); ++iter){
		if (this->edges.at( (*iter)->get_name() ).parents.empty()){
			root_list.push_back(*iter);
		}
	}
	// root events are sorted for easier manipulation
	root_list.sort(Event_comparator());
	return root_list;
}



/*
 * This method output the queue (order in which events are processed) according to the structure of the model
 * This queue is used both for the order of iteration and the design of the marginal array.
 */
queue<shared_ptr<Rec_Event>> Model_Parms::get_model_queue() const{

	list<shared_ptr<Rec_Event>> model_roots = this->get_roots();
	list<shared_ptr<Rec_Event>> events_copy_list = this->events;


	queue<shared_ptr<Rec_Event>> model_queue = *(new queue<shared_ptr<Rec_Event>>); //FIXME nonsense new


	//if all events are root they are independent and thus the queue is sorted only by priority
	if(events_copy_list.size() == model_roots.size()){
		//root list is already sorted by priority
		for(list<shared_ptr<Rec_Event>>::const_iterator iter=model_roots.begin(); iter!=model_roots.end(); ++iter){
			model_queue.push(*iter);
		}
		return model_queue;
	}

	//The root with highest priority is the first event
	events_copy_list.remove(*(model_roots.begin()));
	model_queue.push(*(model_roots.begin()));

	//Keep track of the events already added to the queue
	unordered_map<Rec_Event_name,shared_ptr<Rec_Event>>* processed_events_point = new unordered_map<Rec_Event_name,shared_ptr<Rec_Event>>; //FIXME nonsense new
	(*processed_events_point).insert(make_pair( (*model_roots.begin())->get_name() , *(model_roots.begin()) ));
	model_roots.pop_front();

	events_copy_list.sort(Event_comparator());
	list<shared_ptr<Rec_Event>>::iterator iter = events_copy_list.begin();

	while(!events_copy_list.empty()){

		//Before an event is processed his parents must have been processed, this overrides the priority!

		list<shared_ptr<Rec_Event>> event_parents = this->edges.at( (*iter)->get_name() ).parents;
		bool parents_processed = 1;
		for(list<shared_ptr<Rec_Event>>::const_iterator jiter = event_parents.begin() ; jiter != event_parents.end() ; ++jiter){
			if((*processed_events_point).count( (*jiter)->get_name() ) ==0 ) {parents_processed=0;}
		}
		//If all parents have not been processed, tries the next highest priority event
		if(!parents_processed){
			++iter;
			continue;
		}

		model_queue.push(*iter);
		(*processed_events_point).insert(make_pair( (*iter)->get_name() , (*iter) ));
		events_copy_list.erase(iter);
		events_copy_list.sort(Event_comparator()); //TODO again check this sort function
		iter = events_copy_list.begin();

	}
	delete processed_events_point;
	return model_queue;



}

shared_ptr<Rec_Event> Model_Parms::get_event_pointer(const Rec_Event_name& event_name) const{
	//TODO find something better, might be a bottleneck
	for (list<shared_ptr<Rec_Event>>::const_iterator iter = this->events.begin() ; iter != this->events.end() ; ++iter){
		if ((*iter)->get_name() == event_name){return (*iter);}
	}
	throw runtime_error("Event pointer not found in Model_Parms::get_event_pointer for name:" + event_name);
}

unordered_map<tuple<Event_type,Gene_class,Seq_side>, shared_ptr<Rec_Event>> Model_Parms::get_events_map(){
	unordered_map<tuple<Event_type,Gene_class,Seq_side>, shared_ptr<Rec_Event>> events_map;
	for(list<shared_ptr<Rec_Event>>::const_iterator iter = this->events.begin() ; iter != this->events.end() ; ++iter ){
		events_map.emplace(tuple<Event_type,Gene_class,Seq_side>( (*iter)->get_type() , (*iter)->get_class() , (*iter)->get_side() ) , (*iter));
	}
	return events_map;
}

void Model_Parms::write_model_parms(string filename){
	ofstream outfile(filename);
	outfile<<"@Event_list"<<endl;
	for(list<shared_ptr<Rec_Event>>::const_iterator iter=events.begin() ; iter != events.end() ; ++iter){
		(*iter)->write2txt(outfile);
	}
	outfile<<"@Edges"<<endl;
	for(list<shared_ptr<Rec_Event>>::const_iterator iter=events.begin() ; iter != events.end() ; ++iter){
		list<shared_ptr<Rec_Event>> children = edges[(*iter)->get_name()].children;
		for(list<shared_ptr<Rec_Event>>::const_iterator jiter=children.begin() ; jiter!=children.end() ; ++jiter){
			outfile<<"%"<<(*iter)->get_name()<<";"<<(*jiter)->get_name()<<endl;
		}
	}
	outfile<<"@ErrorRate"<<endl;
	error_rate->write2txt(outfile);
}

void Model_Parms::read_model_parms(string filename){
	ifstream infile(filename);
	if(!infile){
		//Throw exception
		throw runtime_error("File not found : "+filename);
	}
	string line_str;
	getline(infile,line_str);
	if(line_str == string("@Event_list")){
		getline(infile,line_str);
		while(line_str[0] == '#'){
			size_t semicolon_index = line_str.find(";",0);
			string event = line_str.substr(1,semicolon_index-1);
			size_t next_semicolon_index = line_str.find(";",semicolon_index+1);
			string event_class_str = line_str.substr(semicolon_index+1,(next_semicolon_index - semicolon_index - 1));
			Gene_class event_class;
			if(event_class_str == "V_gene"){event_class = V_gene;}
			else if(event_class_str == "VD_genes"){event_class = VD_genes;}
			else if(event_class_str == "D_gene"){event_class = D_gene;}
			else if(event_class_str == "DJ_gene"){event_class = DJ_genes;}
			else if(event_class_str == "VJ_gene"){event_class = VJ_genes;}
			else if(event_class_str == "J_gene"){event_class = J_gene;}
			else if(event_class_str == "VDJ_genes"){event_class = VDJ_genes;}
			else if(event_class_str == "Undefined_gene"){event_class = Undefined_gene;}
			else{
				throw runtime_error("Unknown Gene_class in model file: " + filename);
			}
			semicolon_index = next_semicolon_index;
			next_semicolon_index = line_str.find(";",semicolon_index+1);
			string event_side_str = line_str.substr(semicolon_index+1 , (next_semicolon_index - semicolon_index - 1));
			Seq_side event_side;
			if(event_side_str == "Five_prime"){event_side = Five_prime;}
			else if(event_side_str == "Three_prime"){event_side = Three_prime;}
			else if(event_side_str == "Undefined_side"){event_side = Undefined_side;}
			else{
				throw runtime_error("Unknown event_side in file: " + filename);
			}

			semicolon_index = next_semicolon_index;
			next_semicolon_index = line_str.find(";",semicolon_index+1);
			int priority;
			string nickname;
			if(next_semicolon_index==string::npos){
				//For retro-compatibility purposes
				priority = stoi(line_str.substr(semicolon_index+1,string::npos));
			}
			else{
				//Read the nickname along with the priority
				priority = stoi(line_str.substr(semicolon_index+1,next_semicolon_index-1));
				nickname = line_str.substr(next_semicolon_index+1,string::npos);
			}


			cout<<event<<" read"<<endl;
			if(event == string("Insertion")){
				unordered_map<string,Event_realization> event_realizations = *(new unordered_map<string,Event_realization>()); //FIXME nonsense new
				getline(infile,line_str);
				while(line_str[0]=='%'){
					semicolon_index = line_str.find(";",0);
					int value_int = stoi(line_str.substr(1,semicolon_index));
					int index = stoi(line_str.substr(semicolon_index+1,string::npos));
					event_realizations.emplace(pair<string,Event_realization>(to_string(value_int) , Event_realization(to_string(value_int) , value_int , "",Int_Str() , index)));
					getline(infile,line_str);
				}
				//TODO check this for enum writing
				//Insertion new_event = Insertion(event_class , event_side , event_realizations);
				shared_ptr<Insertion> new_event_p =  shared_ptr<Insertion> (new Insertion(event_class  , event_realizations));
				new_event_p->set_priority(priority);
				new_event_p->set_nickname(nickname);
				this->add_event(new_event_p);
			}
			else if(event == string("Deletion")){
				unordered_map<string,Event_realization> event_realizations = *(new unordered_map<string,Event_realization>()); //FIXME nonsense new
				getline(infile,line_str);
				while(line_str[0]=='%'){
					semicolon_index = line_str.find(";",0);
					int value_int = stoi(line_str.substr(1,semicolon_index));
					int index = stoi(line_str.substr(semicolon_index+1,string::npos));
					event_realizations.emplace(pair<string,Event_realization>(to_string(value_int) , Event_realization(to_string(value_int) , value_int , "",Int_Str() , index))) ;
					getline(infile,line_str);
				}
				//TODO check this for enum writing
				//Deletion new_event = Deletion(event_class , event_side , event_realizations);
				shared_ptr<Deletion> new_event_p =shared_ptr<Deletion> (new Deletion(event_class , event_side , event_realizations));
				new_event_p->set_priority(priority);
				new_event_p->set_nickname(nickname);
				this->add_event(new_event_p);
			}
			else if(event == string("GeneChoice")){
				unordered_map<string,Event_realization> event_realizations = *(new unordered_map<string,Event_realization>()); //FIXME nonsense new
				getline(infile,line_str);
				while(line_str[0]=='%'){
					semicolon_index = line_str.find(";",0);
					string name = line_str.substr(1,semicolon_index-1);
					next_semicolon_index = line_str.find(";",semicolon_index+1);
					string value_str = line_str.substr(semicolon_index+1 , (next_semicolon_index - semicolon_index -1) );
					string test = line_str.substr(next_semicolon_index+1,string::npos);
					int index = stoi(line_str.substr(next_semicolon_index+1,string::npos));
					event_realizations.emplace(pair<string,Event_realization>(name,Event_realization(name , INT16_MAX , value_str , nt2int(value_str) , index)));
					getline(infile,line_str);
				}
				//TODO check this for enum writing
				//Gene_choice new_event = Gene_choice(event_class , event_side , event_realizations);
				shared_ptr<Gene_choice> new_event_p = shared_ptr<Gene_choice> (new Gene_choice(event_class  , event_realizations));//TODO construct event before and use add realization instead?
				new_event_p->set_priority(priority);
				new_event_p->set_nickname(nickname);
				this->add_event(new_event_p);
			}
			else if(event== string("DinucMarkov")){
				shared_ptr<Dinucl_markov> new_event_p = shared_ptr<Dinucl_markov> (new Dinucl_markov(event_class));
				new_event_p->set_priority(priority);
				new_event_p->set_nickname(nickname);
				this->add_event(new_event_p);
				getline(infile,line_str);
				while(line_str[0]=='%'){
					getline(infile,line_str);
				}
			}
			else{
				throw runtime_error(event + " event is not implemented (thrown by Model_Parms::read_model_parms");
			}



		}
	}
	else{
		throw runtime_error("Unknown format for model_parms file");
	}
	if(line_str == string("@Edges")){
		getline(infile,line_str);
		while(line_str[0] == '%'){
			size_t semicolon_index = line_str.find(";",0);
			string parent_name = line_str.substr(1,semicolon_index-1);
			string child_name = line_str.substr(semicolon_index+1 , string::npos);
			edges[parent_name].children.emplace_back(get_event_pointer(child_name));
			edges[child_name].parents.emplace_back(get_event_pointer(parent_name));
			getline(infile , line_str);
		}
	}
	else{
		throw runtime_error("Unknown format for model file: " + filename);
	}
	if(line_str == string("@ErrorRate")){
		getline(infile,line_str);
		if(line_str == string("#SingleErrorRate")){
			getline(infile,line_str);
			shared_ptr<Single_error_rate> err_rate_p = shared_ptr<Single_error_rate> (new Single_error_rate(stod(line_str)));
			this->error_rate = err_rate_p;
		}
		else{
			throw runtime_error("Unknown Error_rate type in model file: " + filename);
		}

	}
	else{
		throw runtime_error("Unknown format for model file: " + filename);
	}


}
