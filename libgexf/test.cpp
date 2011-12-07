#include <stdlib.h>

// if libgexf is installed
#include <libgexf/libgexf.h>

void create() {
  
  libgexf::GEXF *gexf = new libgexf::GEXF();

  libgexf::DirectedGraph& graph = gexf->getDirectedGraph();

  // nodes
  graph.addNode("0");
  graph.addNode("1");

  // edges
  graph.addEdge("0", "0", "1");

  // node labels
  libgexf::Data& data = gexf->getData();
  data.setNodeLabel("0", "Hello");
  data.setNodeLabel("1", "world");

  // attributes
  data.addNodeAttributeColumn("0", "foo", "boolean");
  data.setNodeAttributeDefault("0", "false");
  data.setNodeValue("1", "0", "true");

  // meta data
  libgexf::MetaData& meta = gexf->getMetaData();
  meta.setCreator("The Hitchhiker's Guide to the Galaxy");
  meta.setDescription("The answer is 42.");

  // write gexf file
  libgexf::FileWriter *writer = new libgexf::FileWriter();
  writer->init("life.gexf", gexf);
  writer->write();
}

int main(void) {
  create();
  return 0;
}
