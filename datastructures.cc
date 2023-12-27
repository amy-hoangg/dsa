// Datastructures.cc
//
// Student name:
// Student email:
// Student number:

#include "datastructures.hh"

#include <random>

#include <cmath>

std::minstd_rand rand_engine; // Reasonably quick pseudo-random generator

template <typename Type>
Type random_in_range(Type start, Type end)
{
    auto range = end-start;
    ++range;

    auto num = std::uniform_int_distribution<unsigned long int>(0, range-1)(rand_engine);

    return static_cast<Type>(start+num);
}

// Modify the code below to implement the functionality of the class.
// Also remove comments from the parameter names when you implement
// an operation (Commenting out parameter name prevents compiler from
// warning about unused parameters on operations you haven't yet implemented.)

Datastructures::Datastructures() {
}

Datastructures::~Datastructures()
{
    // Write any cleanup you need here
}

unsigned int Datastructures::get_affiliation_count()
{
    return affiliationsMapContainer_.size();
}

void Datastructures::clear_all()
{
    //clear(): Removes all elements from the vector (which are destroyed), leaving the container with a size of 0.
    affiliationsMapContainer_.clear();
    publicationsMapContainer_.clear();

    // Define a flag to check if the data has been sorted
    affiliationsSortedFlag = false;
    distancesSortedFlag = false;


    // Create a vector of pairs to hold affiliation names and IDs
    nameIDPairs.clear();
    coordIDMap.clear();
    distanceIDMap.clear();
    connectionsMap.clear();
}

std::vector<AffiliationID> Datastructures::get_all_affiliations() {
    std::vector<AffiliationID> allAffiliations;
    allAffiliations.reserve(affiliationsMapContainer_.size()); // Reserve space for efficiency

    std::transform(
        affiliationsMapContainer_.begin(), affiliationsMapContainer_.end(),
        std::back_inserter(allAffiliations),
        [](const auto& pair) { return pair.first; }
    );

    return allAffiliations;
}

bool Datastructures::add_affiliation(AffiliationID id, const Name &name, Coord xy)
{
    if (affiliationsMapContainer_.find(id) != affiliationsMapContainer_.end()) {
        return false; // ID already exists, return false
    }

    Affiliation new_affiliation(id, name, xy);
    affiliationsMapContainer_[id] = new_affiliation;

    // Add name and ID as a pair to the nameIDPairs vector
    nameIDPairs.push_back(std::make_pair(name, id));

    // Update coordIDMap with the new affiliation
    coordIDMap[xy] = id; // Insert into coordIDMap using coordinates (xy) as key

    // Calculate distance between the new affiliation's coordinates and origin (0, 0)
    double distance = std::sqrt(xy.x * xy.x + xy.y * xy.y);

    // Store the distance along with the ID in the distanceIDMap vector
    distanceIDMap.push_back(std::make_pair(distance, id));

    affiliationsSortedFlag = false;
    distancesSortedFlag = false;

    return true; // Affiliation added successfully
}

Name Datastructures::get_affiliation_name(AffiliationID id)
{
    // Check if an affiliation with the given ID exists
    auto it = affiliationsMapContainer_.find(id);
    if (it != affiliationsMapContainer_.end()) {
        return it->second.name; // Return the name of the affiliation
    }

    return NO_NAME; // Return NO_NAME if the affiliation doesn't exist
}

Coord Datastructures::get_affiliation_coord(AffiliationID id)
{
    // Check if an affiliation with the given ID exists
    auto it = affiliationsMapContainer_.find(id);
    if (it != affiliationsMapContainer_.end()) {
        return it->second.coord; // Return the name of the affiliation
    }

    return NO_COORD; // Return NO_NAME if the affiliation doesn't exist
}

std::vector<AffiliationID> Datastructures::get_affiliations_alphabetically() {
    if (!affiliationsSortedFlag) {
        // Sort the affiliations only if they have not been sorted before
        std::sort(nameIDPairs.begin(), nameIDPairs.end());

        affiliationsSortedFlag = true; // Update the flag to indicate sorting has been done
    }

    // Extract sorted IDs
    std::vector<AffiliationID> sortedIDs;
    sortedIDs.reserve(nameIDPairs.size());

    for (const auto& pair : nameIDPairs) {
        sortedIDs.push_back(pair.second);
    }

    return sortedIDs;
}

std::vector<AffiliationID> Datastructures::get_affiliations_distance_increasing() {
    if (!distancesSortedFlag) {
        // Sort the affiliations only if they have not been sorted before
        std::sort(distanceIDMap.begin(), distanceIDMap.end(), [&](const auto& a, const auto& b) {
            // Compare distances first
            if (std::abs(a.first - b.first) < 1e-9) { // Check if distances are approximately equal
                // If distances are the same, smaller y-value comes first
                return affiliationsMapContainer_[a.second].coord.y < affiliationsMapContainer_[b.second].coord.y;
            }
            return a.first < b.first; // Sort based on distance if not equal
        });

        distancesSortedFlag = true; // Update the flag to indicate sorting has been done
    }

    // Extract sorted IDs
    std::vector<AffiliationID> sortedIDs;
    sortedIDs.reserve(distanceIDMap.size());

    for (const auto& pair : distanceIDMap) {
        sortedIDs.push_back(pair.second);
    }

    return sortedIDs;
}

AffiliationID Datastructures::find_affiliation_with_coord(Coord xy) {
    auto it = coordIDMap.find(xy); // Using find directly for lookup

    if (it != coordIDMap.end()) {
        return it->second; // Return the found AffiliationID
    }

    return NO_AFFILIATION; // Return NO_AFFILIATION if not found
}

bool Datastructures::change_affiliation_coord(AffiliationID id, Coord newcoord)
{
    auto it = affiliationsMapContainer_.find(id);

    if (it != affiliationsMapContainer_.end()) {
        it->second.coord = newcoord;

        for (auto it = coordIDMap.begin(); it != coordIDMap.end(); ) {
            if (it->second == id) {
                it = coordIDMap.erase(it); // Remove current entry

                coordIDMap[newcoord] = id;
            } else {
                ++it;
            }
        }


        // Recalculate distances and update distanceIDMap
        for (auto& pair : distanceIDMap) {
            if (pair.second == id) {
                double distance = std::sqrt(newcoord.x * newcoord.x + newcoord.y * newcoord.y);
                pair.first = distance;
            }
        }

        distancesSortedFlag = false; // Update the flag to indicate distances are not sorted
        return true;
    }

    return false;
}

bool Datastructures::add_publication(PublicationID id, const Name &name, Year year, const std::vector<AffiliationID>& affiliations)
{
    // Check if the publication with the given ID exists
    if (publicationsMapContainer_.find(id) != publicationsMapContainer_.end()) {
        return false; // Publication with the same ID already exists
    }

    // Create a new publication object
    Publication new_publication(id, name, year, affiliations);

    // Update the references for the affiliations
    for (const AffiliationID& affiliationID : affiliations) {
        auto affiliationIt = affiliationsMapContainer_.find(affiliationID);
        if (affiliationIt != affiliationsMapContainer_.end()) {
            affiliationIt->second.publications_produced.push_back(id);
        }
    }

    // Add the new publication to the container
    publicationsMapContainer_[id] = new_publication;

    // Update connections between affiliations based on the new publication
    for (size_t i = 0; i < affiliations.size(); ++i) {
        for (size_t j = i + 1; j < affiliations.size(); ++j) {
            AffiliationID aff1 = affiliations[i];
            AffiliationID aff2 = affiliations[j];

            // Check if a connection already exists between these affiliations
            bool connection_exists = false;

            for (Connection& connection : connectionsMap[aff1]) {
                if ((connection.aff1 == aff1 && connection.aff2 == aff2) ||
                    (connection.aff1 == aff2 && connection.aff2 == aff1)) {

                    // Increase the weight of the existing connection
                    connection.weight++;
                    connection_exists = true;
                    break;
                }
            }

            for (Connection& connection : connectionsMap[aff2]) {
                if ((connection.aff1 == aff1 && connection.aff2 == aff2) ||
                    (connection.aff1 == aff2 && connection.aff2 == aff1)) {

                    // Increase the weight of the existing connection
                    connection.weight++;
                    connection_exists = true;
                    break;
                }
            }

            if (!connection_exists) {
                // Create a new connection if no existing connection was found
                Connection new_connection;
                if (aff1 < aff2) {
                    new_connection.aff1 = aff1;
                    new_connection.aff2 = aff2;
                } else {
                    new_connection.aff1 = aff2;
                    new_connection.aff2 = aff1;
                }
                new_connection.weight = calculateWeight(aff1, aff2); // Calculate weight for a new connection

                // Add the new connection to the connectionsMap map for both affiliations
                connectionsMap[aff1].push_back(new_connection);
                connectionsMap[aff2].push_back(new_connection);
            }
        }
    }

    return true;
}

std::vector<PublicationID> Datastructures::all_publications()
{
    std::vector<PublicationID> allPublications;
    for (const auto& entry : publicationsMapContainer_) {
        allPublications.push_back(entry.first);
    }
    return allPublications;
}

Name Datastructures::get_publication_name(PublicationID id)
{
    // Check if an affiliation with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it != publicationsMapContainer_.end()) {
        return it->second.title; // Return the name of the affiliation
    }

    return NO_NAME; // Return NO_NAME if the affiliation doesn't exist
}

Year Datastructures::get_publication_year(PublicationID id)
{
    // Check if an publicarion with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it != publicationsMapContainer_.end()) {
        return it->second.publicationYear; // Return the name of the affiliation
    }

    return NO_YEAR; // Return NO_NAME if the affiliation doesn't exist
}

std::vector<AffiliationID> Datastructures::get_affiliations(PublicationID id)
{
    // Check if an publicarion with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it != publicationsMapContainer_.end()) {
        return it->second.affiliations_produced; // Return the name of the affiliation
    }

    return {NO_AFFILIATION};
}

bool Datastructures::add_reference(PublicationID id, PublicationID parentid)
{
    auto it_childid = publicationsMapContainer_.find(id);
    auto it_parentid = publicationsMapContainer_.find(parentid);

    if (it_childid != publicationsMapContainer_.end() && it_parentid != publicationsMapContainer_.end()) {

        it_parentid->second.publications_reference_to.push_back(id);
        it_childid->second.publication_referenced_by = parentid;

        return true;
    }

    return false;
}

std::vector<PublicationID> Datastructures::get_direct_references(PublicationID id)
{
    // Check if an publicarion with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it != publicationsMapContainer_.end()) {
        return it->second.publications_reference_to; // Return the name of the affiliation
    }

    return {NO_PUBLICATION};
}

bool Datastructures::add_affiliation_to_publication(AffiliationID affiliationid, PublicationID publicationid)
{
    auto it_affiliation = affiliationsMapContainer_.find(affiliationid);
    auto it_publication = publicationsMapContainer_.find(publicationid);

    if (it_affiliation != affiliationsMapContainer_.end() && it_publication != publicationsMapContainer_.end()) {

        it_affiliation->second.publications_produced.push_back(publicationid);
        it_publication->second.affiliations_produced.push_back(affiliationid);

        // Update connection map based on shared publications
         const std::vector<AffiliationID>& affiliations = it_publication->second.affiliations_produced;
         for (size_t i = 0; i < affiliations.size(); ++i) {
             for (size_t j = i + 1; j < affiliations.size(); ++j) {
                 AffiliationID aff1 = affiliations[i];
                 AffiliationID aff2 = affiliations[j];

                 Weight weight = calculateWeight(aff1, aff2);

                 // Check if the calculated weight is greater than zero to create or increase the connection
                 if (weight > 0) {
                     // Check if a connection already exists between these affiliations
                     bool connection_exists = false;

                     for (Connection& connection : connectionsMap[aff1]) {
                         if ((connection.aff1 == aff1 && connection.aff2 == aff2) ||
                             (connection.aff1 == aff2 && connection.aff2 == aff1)) {

                             // Increase the weight of the existing connection
                             connection.weight = weight;
                             connection_exists = true;
                             break;
                         }
                     }

                     for (Connection& connection : connectionsMap[aff2]) {
                         if ((connection.aff1 == aff1 && connection.aff2 == aff2) ||
                             (connection.aff1 == aff2 && connection.aff2 == aff1)) {

                             // Increase the weight of the existing connection
                             connection.weight = weight;
                             connection_exists = true;
                             break;
                         }
                     }

                     if (!connection_exists) {
                         // Create a new connection if no existing connection was found
                         Connection new_connection;
                         if (aff1 < aff2) {
                             new_connection.aff1 = aff1;
                             new_connection.aff2 = aff2;
                         } else {
                             new_connection.aff1 = aff2;
                             new_connection.aff2 = aff1;
                         }
                         new_connection.weight = calculateWeight(aff1, aff2); // Calculate weight for a new connection

                         // Add the new connection to the connectionsMap map for both affiliations
                         connectionsMap[aff1].push_back(new_connection);
                         connectionsMap[aff2].push_back(new_connection);
                     }
                 }
             }
         }


        return true;
    }

    return false;
}

std::vector<PublicationID> Datastructures::get_publications(AffiliationID id)
{
    // Check if an publicarion with the given ID exists
    auto it = affiliationsMapContainer_.find(id);
    if (it != affiliationsMapContainer_.end()) {
        return it->second.publications_produced; // Return the name of the affiliation
    }

    return {NO_PUBLICATION};
}

PublicationID Datastructures::get_parent(PublicationID id)
{
    // Check if a publication with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it != publicationsMapContainer_.end()) {
        PublicationID parentID = it->second.publication_referenced_by;

        // Check if the publication has a parent reference
        // Check if the publication has a parent reference
        if (parentID != NO_PUBLICATION && publicationsMapContainer_.count(parentID) > 0) {
            return parentID;
        }
        else {
            return NO_PUBLICATION;
        }
    }

    return NO_PUBLICATION;
}

std::vector<std::pair<Year, PublicationID>> Datastructures::get_publications_after(AffiliationID affiliationid, Year year)
{
    std::vector<std::pair<Year, PublicationID>> result;

    // Check if the affiliation with the given ID exists in affiliationsMapContainer_
    if (affiliationsMapContainer_.find(affiliationid) != affiliationsMapContainer_.end())
    {
        // Iterate through the publications produced by the affiliation
        const Affiliation& affiliation = affiliationsMapContainer_[affiliationid]; //get this affiliation
        for (const PublicationID& publicationid : affiliation.publications_produced)
        {
            // Check if the publication with the given ID exists in publicationsMapContainer_
            if (publicationsMapContainer_.find(publicationid) != publicationsMapContainer_.end())
            {
                const Publication& publication = publicationsMapContainer_[publicationid]; //get this publication
                // Check if the publication's year is at or after the specified year
                if (publication.publicationYear >= year)
                {
                    // Add the publication's year and ID to the result vector
                    result.emplace_back(publication.publicationYear, publicationid);
                }
            }
        }

        // Sort the result vector by publication year and then by publication ID
        std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            if (a.first != b.first) {
                return a.first < b.first; //sort by year
            }
            return a.second < b.second; //sort by id
        });
    }
    else
    {
        // If the affiliation does not exist, return {NO_YEAR, NO_PUBLICATION}
        result.emplace_back(NO_YEAR, NO_PUBLICATION);
    }
    return result;
}

std::vector<PublicationID> Datastructures::get_referenced_by_chain(PublicationID id)
{
    auto it = publicationsMapContainer_.find(id);

    if (it != publicationsMapContainer_.end())
    {
        std::vector<PublicationID> result;
        std::unordered_set<PublicationID> visited;

        std::function<void(PublicationID)> dfs = [&](PublicationID current_id) {
            visited.insert(current_id);

            if (publicationsMapContainer_.find(current_id) != publicationsMapContainer_.end()) {
                const Publication& publication = publicationsMapContainer_[current_id];

                // Exclude the original publication from the result
                if (current_id != id) {
                    result.push_back(current_id);
                }

                // Check if there is a parent publication that references the current publication
                PublicationID parent_id = publication.publication_referenced_by;
                if (visited.find(parent_id) == visited.end() && parent_id != NO_PUBLICATION) {
                    dfs(parent_id);
                }
            }
        };

        dfs(id);

        return result;
    }

    return {NO_PUBLICATION};
}

std::vector<PublicationID> Datastructures::get_all_references(PublicationID id)
{
    // Check if the publication with the given ID exists
    auto it = publicationsMapContainer_.find(id);
    if (it == publicationsMapContainer_.end())
    {
        // If the publication doesn't exist, return a vector with a single item NO_PUBLICATION
        return {NO_PUBLICATION};
    }

    // If the publication exists, perform a DFS to find all references
    std::vector<PublicationID> result;
    std::unordered_set<PublicationID> visited;

    // Perform a depth-first search to find publications that reference the given publication
    std::function<void(PublicationID)> dfs = [&](PublicationID current_id) {
        visited.insert(current_id);
        const Publication& publication = publicationsMapContainer_[current_id];

        // Iterate through publications that reference the current publication
        for (PublicationID referenced_id : publication.publications_reference_to)
        {
            // If the referenced publication hasn't been visited yet, continue the DFS
            if (visited.find(referenced_id) == visited.end())
            {
                dfs(referenced_id);
            }
        }
        // Exclude the original publication from the result
        if (current_id != id)
        {
            result.push_back(current_id);
        }
    };

    dfs(id);
    return result;
}

std::vector<AffiliationID> Datastructures::get_affiliations_closest_to(Coord xy)
{
    // Create a vector to store affiliation IDs
    std::vector<AffiliationID> affiliationIDs;

    // Populate the vector with affiliation IDs
    for (const auto& entry : affiliationsMapContainer_)
    {
        affiliationIDs.push_back(entry.first);
    }

    // Sort the vector based on affiliation coordinates
    std::sort(affiliationIDs.begin(), affiliationIDs.end(),
              /*lambda function*/
              [this, xy](const AffiliationID& id1, const AffiliationID& id2) {
                  const Affiliation& affiliation1 = affiliationsMapContainer_.at(id1);
                  const Affiliation& affiliation2 = affiliationsMapContainer_.at(id2);

                  const Distance distance1 = sqrt(pow(affiliation1.coord.x - xy.x, 2) + pow(affiliation1.coord.y - xy.y, 2));
                  const Distance distance2 = sqrt(pow(affiliation2.coord.x - xy.x, 2) + pow(affiliation2.coord.y - xy.y, 2));

                  if (distance1 == distance2)
                  {
                      return affiliation1.coord.y < affiliation2.coord.y;
                  }
                  return distance1 < distance2;
              });

    // Check if there are no affiliations
    if (affiliationIDs.empty())
    {
        return {};
    }

    // Check if there are less than three affiliations
    if (affiliationIDs.size() < 3)
    {
        return affiliationIDs;
    }

    // Return the three closest affiliations
    return {affiliationIDs[0], affiliationIDs[1], affiliationIDs[2]};
}

bool Datastructures::remove_affiliation(AffiliationID id)
{
    // Check if the affiliation with the given ID exists
    auto it = affiliationsMapContainer_.find(id);
    if (it == affiliationsMapContainer_.end())
    {
        return false; // Affiliation doesn't exist, return false
    }

    // Affiliation exists, so remove it from affiliationsMapContainer_
    affiliationsMapContainer_.erase(it);

    // Remove the affiliation from nameIDPairs
    auto nameIDPairsIt = std::find_if(nameIDPairs.begin(), nameIDPairs.end(),
        [id](const std::pair<std::string, AffiliationID>& pair) { return pair.second == id; });

    if (nameIDPairsIt != nameIDPairs.end())
    {
        nameIDPairs.erase(nameIDPairsIt);
    }

    // Remove the affiliation from coordIDMap
    auto coordIDMapIt = std::find_if(coordIDMap.begin(), coordIDMap.end(),
        [id](const std::pair<Coord, AffiliationID>& pair) { return pair.second == id; });

    if (coordIDMapIt != coordIDMap.end())
    {
        coordIDMap.erase(coordIDMapIt);
    }

    // Remove the affiliation from distanceIDMap
    auto distanceIDMapIt = std::find_if(distanceIDMap.begin(), distanceIDMap.end(),
        [id](const std::pair<double, AffiliationID>& pair) { return pair.second == id; });

    if (distanceIDMapIt != distanceIDMap.end())
    {
        distanceIDMap.erase(distanceIDMapIt);
    }

    // Remove connections related to the removed affiliation
    for (auto &connections : connectionsMap) {
        connections.second.erase(std::remove_if(connections.second.begin(), connections.second.end(),
            [id](const Connection& conn) { return conn.aff1 == id || conn.aff2 == id; }),
            connections.second.end());

        // Also remove connections involving 'id' from the other affiliations involved
        for (auto &other_connection : connections.second) {
            if (other_connection.aff1 == id || other_connection.aff2 == id) {
                AffiliationID other_affiliation = (other_connection.aff1 == id) ? other_connection.aff2 : other_connection.aff1;
                connectionsMap[other_affiliation].erase(
                    std::remove_if(connectionsMap[other_affiliation].begin(), connectionsMap[other_affiliation].end(),
                        [id, other_affiliation](const Connection& conn) {
                            return (conn.aff1 == id && conn.aff2 == other_affiliation) ||
                                   (conn.aff1 == other_affiliation && conn.aff2 == id);
                        }),
                    connectionsMap[other_affiliation].end());
            }
        }
    }

    return true; // Affiliation removed successfully
}

PublicationID Datastructures::get_closest_common_parent(PublicationID id1, PublicationID id2)
{
    // Check if the publications with the given IDs exist
    auto it1 = publicationsMapContainer_.find(id1);
    auto it2 = publicationsMapContainer_.find(id2);

    if (it1 == publicationsMapContainer_.end() || it2 == publicationsMapContainer_.end()) {
        // If either publication doesn't exist, return NO_PUBLICATION
        return NO_PUBLICATION;
    }

    // Get the chains of publications that reference id1 and id2
    std::vector<PublicationID> chain1 = get_referenced_by_chain(id1);
    std::vector<PublicationID> chain2 = get_referenced_by_chain(id2);

    // Find the closest common parent in the chains
    PublicationID closest_common_parent = NO_PUBLICATION;
    for (PublicationID parent1 : chain1) {
        for (PublicationID parent2 : chain2) {
            if (parent1 == parent2) {
                // Found a common parent, update closest_common_parent
                closest_common_parent = parent1;
                return closest_common_parent;
            }
        }
    }

    // If there's no common parent, return NO_PUBLICATION
    return NO_PUBLICATION;
}

bool Datastructures::remove_publication(PublicationID publicationid)
{
    // Check if the publication with the given ID exists
    auto it = publicationsMapContainer_.find(publicationid);

    // If the publication doesn't exist, return false
    if (it == publicationsMapContainer_.end()) {
        return false;
    }

    // Remove the publication from publicationsMapContainer
    publicationsMapContainer_.erase(it);

    // Iterate through affiliations and remove the reference to the publication
    for (auto& affiliation : affiliationsMapContainer_) {
        auto& publications_produced = affiliation.second.publications_produced;
        publications_produced.erase(std::remove(publications_produced.begin(), publications_produced.end(), publicationid), publications_produced.end());
    }

    // Iterate through other publications and remove references
    for (auto& publication : publicationsMapContainer_) {

        // For this publication, if it is referenced by one publication (in its publication_referenced_by),
        // remove publicationid from this publication's publication_referenced_by.
        auto& references = publication.second.publications_reference_to;
        references.erase(std::remove(references.begin(), references.end(), publicationid), references.end());

        // For this publication, if it references other publications (in its publications_reference_to vector),
        // remove publicationid from each of these publications' PublicationID referenced_by.
        if (publication.second.publication_referenced_by == publicationid) {
            publication.second.publication_referenced_by = NO_PUBLICATION;
        }
    }

    // Retrieve affiliations linked to this publication
    auto& affiliations_produced = it->second.affiliations_produced;

    // Iterate through affected affiliations and update connections or remove when weight is 0
    for (auto& affiliationID : affiliations_produced) {
        auto& affiliation = affiliationsMapContainer_[affiliationID];
        auto& publications_produced = affiliation.publications_produced;

        publications_produced.erase(
            std::remove(publications_produced.begin(), publications_produced.end(), publicationid),
            publications_produced.end()
        );

        // Update or remove connections between affiliations when weight is zero
        auto& connections = connectionsMap[affiliationID];
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                           [&](const Connection& connection) {
                               return connection.weight == 0 &&
                                      (connection.aff1 == affiliationID || connection.aff2 == affiliationID);
                           }),
            connections.end()
        );
    }

    return true;
}

Weight Datastructures::calculateWeight(AffiliationID id1, AffiliationID id2) {
    // Retrieve publications for both affiliations
    std::vector<PublicationID> publications1 = get_publications(id1);
    std::vector<PublicationID> publications2 = get_publications(id2);

    // Sort the publication lists
    std::sort(publications1.begin(), publications1.end());
    std::sort(publications2.begin(), publications2.end());

    // Initialize a vector to store shared publications
    std::vector<PublicationID> shared;
    std::set_intersection(
        publications1.begin(), publications1.end(),
        publications2.begin(), publications2.end(),
        std::back_inserter(shared)
    );

    // Count the number of shared publications
    return shared.size();
}

std::vector<Connection> Datastructures::get_connected_affiliations(AffiliationID id) {
    std::vector<Connection> connectedAffiliations;

    // Check if the given affiliation ID exists in affiliationsMapContainer_
    if (affiliationsMapContainer_.find(id) == affiliationsMapContainer_.end()) {
        return connectedAffiliations; // Empty vector: affiliation ID does not exist
    }

    // Iterate through connectionsMap to find connections related to the given affiliation ID
    for (const auto& connection : connectionsMap) {
        if (connection.first == id) {
            for (const auto& conn : connection.second) {
                // Check if the connection has the given ID as aff1
                if (conn.aff1 == id) {
                    connectedAffiliations.push_back(conn);
                }
                else if (conn.aff2 == id) {
                    // If the given ID is aff2, swap aff1 with aff2 and add to connectedAffiliations
                    Connection swappedConnection = conn;
                    std::swap(swappedConnection.aff1, swappedConnection.aff2);
                    connectedAffiliations.push_back(swappedConnection);
                }
            }
        }
    }

    return connectedAffiliations;
}
// Custom comparison function for Connection
struct ConnectionComparator {
    bool operator()(const Connection& c1, const Connection& c2) const {
        // Compare based on some criteria considering aff1, aff2, and weight
        if (c1.aff1 != c2.aff1)
            return c1.aff1 < c2.aff1;
        else if (c1.aff2 != c2.aff2)
            return c1.aff2 < c2.aff2;
        else
            return c1.weight < c2.weight;
    }
};

std::vector<Connection> Datastructures::get_all_connections() {
    std::vector<Connection> allConnections;
    std::set<Connection, ConnectionComparator> uniqueConnections;

    // Iterate through connectionsMap to gather all connections
    for (const auto& pair : connectionsMap) {
        for (const auto& connection : pair.second) {
            // Check if the connection already exists based on the custom comparator
            if (uniqueConnections.find(connection) == uniqueConnections.end()) {
                // Add the unique connection to allConnections
                allConnections.push_back(connection);
                uniqueConnections.insert(connection);
            }
        }
    }

    return allConnections;
}

std::vector<Connection> Datastructures::getModifiedPath(const std::vector<Connection>& path, AffiliationID source) {
    std::vector<Connection> modifiedPath = path;

    // Modify the first connection if aff1 doesn't match the source affiliation
    if (!modifiedPath.empty() && modifiedPath[0].aff1 != source) {
        std::swap(modifiedPath[0].aff1, modifiedPath[0].aff2);
    }

    // Modify subsequent connections based on affiliation conditions
    for (size_t i = 1; i < modifiedPath.size(); ++i) {
        if (modifiedPath[i].aff1 != modifiedPath[i - 1].aff2) {
            std::swap(modifiedPath[i].aff1, modifiedPath[i].aff2);
        }
    }

    return modifiedPath;
}

Path Datastructures::get_any_path(AffiliationID source, AffiliationID target) {
        // Initialize visited map
        for (const auto& pair : connectionsMap) {
            visited[pair.first] = false;
        }

        stack.push(source);
        visited[source] = true;

        while (!stack.empty()) {
            AffiliationID current = stack.top();
            stack.pop();

            for (const Connection& connection : connectionsMap[current]) {
                AffiliationID neighbor = (connection.aff1 == current) ? connection.aff2 : connection.aff1;

                if (!visited[neighbor]) {
                    stack.push(neighbor);
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                }
            }
        }

        // Construct path using the parent map
        std::vector<Connection> path;
        std::vector<Connection> new_path;
        AffiliationID current = target;

        while (current != source && visited[current]) {
            AffiliationID prev = parent[current];
            // Find the connection between current and prev
            for (const Connection& connection : connectionsMap[current]) {
                if ((connection.aff1 == current && connection.aff2 == prev) ||
                    (connection.aff1 == prev && connection.aff2 == current)) {
                    path.push_back(connection);
                    break;
                }
            }
            current = prev;
        }

        // Reverse the path to get source to target order
        std::reverse(path.begin(), path.end());


        if (current == source) {
            std::vector<Connection> finalPath = getModifiedPath(path, source);
            return finalPath; // Return the modified path if a valid path is found
        } else {
            return std::vector<Connection>(); // Return an empty vector if no path is found
        }
}

std::vector<Connection> Datastructures::get_path_with_least_affiliations(AffiliationID source, AffiliationID target) {
    // Check if source or target affiliations do not exist
    if (affiliationsMapContainer_.find(source) == affiliationsMapContainer_.end() ||
        affiliationsMapContainer_.find(target) == affiliationsMapContainer_.end()) {
        return {}; // Return empty vector if source or target does not exist
    }

    // Reset visited and parent maps
    visited.clear();
    parent.clear();

    std::queue<AffiliationID> queue;
    queue.push(source);
    visited[source] = true;

    while (!queue.empty()) {
        AffiliationID current = queue.front();
        queue.pop();

        // Check if the target affiliation is reached
        if (current == target) {
            std::vector<Connection> path;
            // Reconstruct the path using the parent map
            while (current != source) {
                AffiliationID prev = parent[current];
                for (const Connection& connection : connectionsMap[prev]) {
                    if ((connection.aff1 == prev && connection.aff2 == current) ||
                        (connection.aff1 == current && connection.aff2 == prev)) {
                        path.push_back(connection);
                        break;
                    }
                }
                current = prev;
            }
            std::reverse(path.begin(), path.end());
            std::vector<Connection> finalPath = getModifiedPath(path, source);
            return finalPath; // Return the modified path if a valid path is found
        }

        //this one is for after not find path
        // Explore connections of the current affiliation
        for (const Connection& connection : connectionsMap[current]) {
            AffiliationID next;
            if (connection.aff1 == current) {
                next = connection.aff2;
            } else {
                next = connection.aff1;
            }

            if (!visited[next]) {
                visited[next] = true;
                parent[next] = current;
                queue.push(next);
            }
        }
    }

    return {}; // Return empty vector if no path is found
}

Path Datastructures::get_path_of_least_friction(AffiliationID source, AffiliationID target)
{
    // Check if source or target affiliations do not exist
    if (affiliationsMapContainer_.find(source) == affiliationsMapContainer_.end() ||
        affiliationsMapContainer_.find(target) == affiliationsMapContainer_.end()) {
        return {}; // Return empty vector if source or target does not exist
    }

    // Reset visited and parent maps
    visited.clear();
    parent.clear();

    std::priority_queue<std::pair<Weight, AffiliationID>> queue; // Use priority queue based on weight
    queue.push({0.0, source}); // Start with source and weight 0
    visited[source] = true;

    std::vector<Connection> maxPath;
    Weight maxWeight = 0.0;

    while (!queue.empty()) {
        auto currentPair = queue.top();
        queue.pop();
        Weight currentWeight = currentPair.first;
        AffiliationID current = currentPair.second;

        // Check if the target affiliation is reached and weight is higher
        if (current == target && currentWeight > maxWeight) {
            maxWeight = currentWeight;
            maxPath.clear();
            // Reconstruct the path using the parent map
            while (current != source) {
                AffiliationID prev = parent[current];
                for (const Connection& connection : connectionsMap[prev]) {
                    if ((connection.aff1 == prev && connection.aff2 == current) ||
                        (connection.aff1 == current && connection.aff2 == prev)) {
                        maxPath.push_back(connection);
                        break;
                    }
                }
                current = prev;
            }
            std::reverse(maxPath.begin(), maxPath.end());
        }

        // Explore connections of the current affiliation
        for (const Connection& connection : connectionsMap[current]) {
            AffiliationID next;
            if (connection.aff1 == current) {
                next = connection.aff2;
            } else {
                next = connection.aff1;
            }

            if (!visited[next]) {
                visited[next] = true;
                parent[next] = current;
                queue.push({currentWeight + connection.weight, next});
            }
        }
    }

    return maxPath; // Return the path with the highest weight
}

PathWithDist Datastructures::get_shortest_path(AffiliationID source, AffiliationID target) {
    // Check if the source and target IDs exist in the affiliations_ map
    if (affiliationsMapContainer_.count(source) == 0 || affiliationsMapContainer_.count(target) == 0) {
        return PathWithDist(); // Return an empty vector if source or target not found
    }

    // Create a set to store the visited affiliations
    std::set<AffiliationID> visited;

    // Create a priority queue to store the next nodes to explore based on distance
    std::priority_queue<std::pair<Distance, AffiliationID>> pq;

    // Map to store the distance from source to each node
    std::unordered_map<AffiliationID, Distance> distance;

    // Map to store the previous node in the shortest path
    std::unordered_map<AffiliationID, Connection> previous;

    // Initialize distances
    distance[source] = 0;

    // Enqueue the source node with distance 0
    pq.push({0, source});

    while (!pq.empty()) {
        AffiliationID current = pq.top().second;
        pq.pop();

        // Check if the current node is the target
        if (current == target) {
            // Reconstruct the path
            PathWithDist result;
            AffiliationID current_node = target;

            while (previous.find(current_node) != previous.end()) {
                Connection connection = previous[current_node];
                // Calculate the distance of the edge by subtracting the distance of the previous node
                Distance edge_distance = distance[current_node] - distance[connection.aff1 == current_node ? connection.aff2 : connection.aff1];
                result.push_back({connection, edge_distance});
                current_node = connection.aff1 == current_node ? connection.aff2 : connection.aff1;
            }

            std::reverse(result.begin(), result.end());
            return result;
        }

        // Mark the current node as visited
        visited.insert(current);

        // Get the connections of the current affiliation
        std::vector<Connection> connections = get_connected_affiliations(current);

        // Loop through the connections
        for (const Connection &connection : connections) {
            // Get the other affiliation ID
            AffiliationID other = (connection.aff1 == current) ? connection.aff2 : connection.aff1;

            // Check if the other ID is not visited
            if (visited.count(other) == 0) {
                // Calculate Euclidean distance inline
                Coord coord1 = affiliationsMapContainer_[current].coord;
                Coord coord2 = affiliationsMapContainer_[other].coord;
                Distance tentative_distance = static_cast<Distance>(
                    std::sqrt(std::pow(coord1.x - coord2.x, 2) + std::pow(coord1.y - coord2.y, 2))
                );

                // Calculate the tentative total distance
                Distance total_distance = distance[current] + tentative_distance;

                // If the tentative total distance is smaller, update the distance and enqueue the neighbor
                if (distance.find(other) == distance.end() || total_distance < distance[other]) {
                    distance[other] = total_distance;
                    previous[other] = connection;
                    pq.push({-total_distance, other});
                }
            }
        }
    }

    // If the loop completes, there is no path from source to target
    return PathWithDist();
}


