######################################################################
#  Copyright (c) 2008-2013, Alliance for Sustainable Energy.  
#  All rights reserved.
#  
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#  
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
######################################################################

# Each user script is implemented within a class that derives from OpenStudio::Ruleset::UserScript
class ConvertGroupsToOpenStudioSpaces < OpenStudio::Ruleset::ModelUserScript

  # override name to return the name of your script
  def name
    return "Convert SketchUp Groups To OpenStudio Spaces"
  end
  
  # returns a vector of arguments, the runner will present these arguments to the user
  # then pass in the results on run
  def arguments(model)
    result = OpenStudio::Ruleset::OSArgumentVector.new
    return result
  end
    
  # override run to implement the functionality of your script
  # model is an OpenStudio::Model::Model, runner is a OpenStudio::Ruleset::UserScriptRunner
  def run(model, runner, user_arguments)
    super(model, runner, user_arguments) # initializes runner for new script

    # get sketchup model
    suModel = Sketchup.active_model
    entities = suModel.active_entities #not using this, running script on selection instead of everything
    selection = suModel.selection

    #make array of groups, and make unique
    groupsAndComponents = []
    selection.each do |entity|
      if entity.class.to_s == "Sketchup::Group" or entity.class.to_s == "Sketchup::ComponentInstance"
        if entity.drawing_interface == nil
          entity.make_unique #this is only needed if a group was copied.
          groupsAndComponents << entity
        else
          puts "#{entity.drawing_interface.model_object.name} is already an OpenStudio Model Object."
        end
      end
    end

    #loop through groups
    groupsAndComponents.each do |groupAndComponent|

      #store name of group
      groupAndComponent_name = groupAndComponent.name #not sure if this works for components

      # make new space
      space = OpenStudio::Space.new
      space.create_model_object
      space.draw_entity
      space.add_observers
      space.add_watcher
      space.model_object.setName(groupAndComponent_name)

      #move group inside of space and explode
      entity = space.entity
      if groupAndComponent.class.to_s == "Sketchup::Group"
        tempGroup = entity.entities.add_instance(groupAndComponent.entities.parent, groupAndComponent.transformation*entity.transformation)
      elsif groupAndComponent.class.to_s == "Sketchup::ComponentInstance"
        tempGroup = entity.entities.add_instance(groupAndComponent.definition.entities.parent, groupAndComponent.transformation*entity.transformation)
      end

      #issue warning if group is not manifold solid
      if not groupAndComponent.manifold?
        puts "*(warning) The geometry in #{space.name} is not a manifold solid, and may not run properly in an EnergyPlus simulation."
      end

      #explode temp copy so observers can classify, then erase original geometry
      tempGroup.explode
      groupAndComponent.erase! #could be nice to through this on a disabled layer instead of deleting it, similar to space diagram and projected geometry

      #seems to handle sub-surfaces well, but there may be some cases that this does not happen.

      #should clean up origins better in new spaces vs. having it us 0,0,0 - todo

      #doesn't support nested groups to be used for shading an interior partition surfaces - todo

    end
    
  end

end

# this call registers your script with the OpenStudio SketchUp plug-in
ConvertGroupsToOpenStudioSpaces.new.registerWithApplication