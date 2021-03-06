package com.yahoo.searchdefinition.processing;

import com.yahoo.config.application.api.DeployLogger;
import com.yahoo.searchdefinition.RankProfile;
import com.yahoo.searchdefinition.RankProfileRegistry;
import com.yahoo.searchdefinition.Search;
import com.yahoo.searchlib.rankingexpression.parser.RankingExpressionParserConstants;
import com.yahoo.vespa.model.container.search.QueryProfiles;

import java.util.HashSet;
import java.util.Set;
import java.util.logging.Level;

/**
 * Issues a warning if some macro has a reserved name. This is not necessarily
 * an error, as a macro can shadow a built-in function.
 *
 * @author lesters
 */
public class ReservedMacroNames extends Processor {

    public ReservedMacroNames(Search search, DeployLogger deployLogger, RankProfileRegistry rankProfileRegistry, QueryProfiles queryProfiles) {
        super(search, deployLogger, rankProfileRegistry, queryProfiles);
    }

    @Override
    public void process() {
        Set<String> reservedNames = getReservedNames();
        for (RankProfile rp : rankProfileRegistry.allRankProfiles()) {
            for (String macroName : rp.getMacros().keySet()) {
                if (reservedNames.contains(macroName)) {
                    deployLogger.log(Level.WARNING, "Macro \"" + macroName + "\" " +
                            "in rank profile \"" + rp.getName() + "\" " +
                            "has a reserved name. This might mean that the macro shadows " +
                            "the built-in function with the same name."
                    );
                }
            }
        }
    }

    private Set<String> getReservedNames() {
        Set<String> names = new HashSet<>();
        for (String token : RankingExpressionParserConstants.tokenImage) {
            String tokenWithoutQuotes = token.substring(1, token.length()-1);
            names.add(tokenWithoutQuotes);
        }
        return names;
    }
}
